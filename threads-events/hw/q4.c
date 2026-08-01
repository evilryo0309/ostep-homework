#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30

// Define client states
#define STATE_UNUSED 0  // Unused
#define STATE_IDLE 1    // Connected, waiting for requests
#define STATE_READING 2 // Reading file in background

// Client context structure
struct client_context
{
    int state;
    int socket_fd;
    int file_fd;
    off_t file_offset; // Current file offset
    struct aiocb cb;   // AIO control block
    char buffer[BUFFER_SIZE];
};

int main()
{
    int server_fd, new_socket, max_sd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set readfds;

    // Initialize clients array
    struct client_context clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].state = STATE_UNUSED;
    }

    // 1. Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    // 2. Bind port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("AIO File Server is listening on port %d...\n", PORT);

    // 4. Event loop
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add IDLE clients to read set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].state == STATE_IDLE)
            {
                FD_SET(clients[i].socket_fd, &readfds);
                if (clients[i].socket_fd > max_sd)
                    max_sd = clients[i].socket_fd;
            }
        }

        // Set select timeout to 10 ms (0.01s)
        // This ensures we can poll AIO status periodically
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select error");
        }

        // --- Phase A: Handle network events (new connections or requests) ---
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
            {
                perror("accept error");
            }
            else
            {
                printf("New connection, socket fd is %d\n", new_socket);
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i].state == STATE_UNUSED)
                    {
                        clients[i].state = STATE_IDLE;
                        clients[i].socket_fd = new_socket;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].state == STATE_IDLE && FD_ISSET(clients[i].socket_fd, &readfds))
            {
                int valread = read(clients[i].socket_fd, clients[i].buffer, BUFFER_SIZE - 1);

                if (valread <= 0)
                {
                    close(clients[i].socket_fd);
                    clients[i].state = STATE_UNUSED;
                }
                else
                {
                    // Parse filename and perform security checks
                    clients[i].buffer[valread] = '\0';
                    clients[i].buffer[strcspn(clients[i].buffer, "\r\n")] = 0;

                    if (strstr(clients[i].buffer, "..") != NULL || clients[i].buffer[0] == '/')
                    {
                        char *err_msg = "Error: Access Denied.\n";
                        send(clients[i].socket_fd, err_msg, strlen(err_msg), 0);
                        continue;
                    }

                    // Open file
                    clients[i].file_fd = open(clients[i].buffer, O_RDONLY);
                    if (clients[i].file_fd < 0)
                    {
                        char *err_msg = "Error: File Not Found.\n";
                        send(clients[i].socket_fd, err_msg, strlen(err_msg), 0);
                    }
                    else
                    {
                        // Prepare initial AIO read
                        clients[i].file_offset = 0;
                        memset(&clients[i].cb, 0, sizeof(struct aiocb));
                        clients[i].cb.aio_fildes = clients[i].file_fd;
                        clients[i].cb.aio_buf = clients[i].buffer;
                        clients[i].cb.aio_nbytes = BUFFER_SIZE;
                        clients[i].cb.aio_offset = clients[i].file_offset;

                        if (aio_read(&clients[i].cb) == -1)
                        {
                            perror("aio_read failed");
                            close(clients[i].file_fd);
                        }
                        else
                        {
                            // Switch state: waiting for background read
                            clients[i].state = STATE_READING;
                            printf("Started async read for file fd %d\n", clients[i].file_fd);
                        }
                    }
                }
            }
        }

        // --- Phase B: Poll AIO status (process background read results) ---
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].state == STATE_READING)
            {
                int err = aio_error(&clients[i].cb);

                if (err == 0)
                {
                    // aio_error == 0 means read completed
                    int bytes_read = aio_return(&clients[i].cb);

                    if (bytes_read > 0)
                    {
                        // Send data read to client
                        send(clients[i].socket_fd, clients[i].buffer, bytes_read, 0);

                        // Update file offset and initiate next background read
                        clients[i].file_offset += bytes_read;
                        clients[i].cb.aio_offset = clients[i].file_offset;

                        if (aio_read(&clients[i].cb) == -1)
                        {
                            perror("aio_read failed on continue");
                        }
                    }
                    else
                    {
                        // bytes_read == 0 means end of file (EOF)
                        printf("Finished serving file fd %d. Closing connection.\n", clients[i].file_fd);
                        close(clients[i].file_fd);
                        close(clients[i].socket_fd);
                        clients[i].state = STATE_UNUSED; // Release client resources
                    }
                }
                else if (err != EINPROGRESS)
                {
                    // If not 0 and not EINPROGRESS, an error occurred
                    perror("AIO read error");
                    close(clients[i].file_fd);
                    close(clients[i].socket_fd);
                    clients[i].state = STATE_UNUSED;
                }
                // If EINPROGRESS (read in progress), do nothing and wait for next loop iteration
            }
        }
    }

    return 0;
}