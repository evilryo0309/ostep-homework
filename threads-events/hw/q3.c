#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 256
#define MAX_CLIENTS 30 // Maximum number of concurrent client connections allowed

int main()
{
    int server_fd, new_socket, client_sockets[MAX_CLIENTS];
    int max_sd, sd, activity, i, valread;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Declare fd_set
    fd_set readfds;

    // Initialize all client_sockets to 0 (0 means unused)
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    // 1. Create TCP Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow quick server restart for development convenience
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 2. Configure Server Address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Select Time Server is listening on port %d...\n", PORT);

    // 5. Event Loop
    while (1)
    {
        // Clear fd_set and add server_fd to it
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add all valid client_sockets to fd_set
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            // Update max_sd because select needs to know the highest FD number
            if (sd > max_sd)
                max_sd = sd;
        }

        // Call select, which blocks until an event occurs on any FD
        // (NULL means no timeout, wait indefinitely)
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0))
        {
            perror("select error");
        }

        // Event A: Server FD activity indicates an incoming connection
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
            {
                perror("accept error");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is: %s, port: %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add the new socket to the array
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    printf("Adding to list of sockets at index %d\n", i);
                    break;
                }
            }

            activity--; // Decrement activity count since we handled one event
        }

        // Event B: Check if an existing client sent data
        for (i = 0; i < MAX_CLIENTS && activity; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Read data
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0)
                {
                    // Reading 0 means client disconnected
                    getpeername(sd, (struct sockaddr *)&client_addr, &client_len);
                    printf("Host disconnected, ip %s, port %d \n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(sd);
                    client_sockets[i] = 0; // Release array slot
                }
                else
                {
                    // 1. Clean string: Incoming client strings usually contain newline characters (\n or \r)
                    // We need to replace them with null terminators \0 to use as a filename
                    buffer[valread] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = 0;

                    // If user pressed Enter directly (empty string), ignore it
                    if (strlen(buffer) == 0)
                    {
                        continue;
                    }

                    printf("Client requested file: %s\n", buffer);

                    // 2. Simple security check: Prevent Path Traversal
                    if (strstr(buffer, "..") != NULL || buffer[0] == '/')
                    {
                        char *err_msg = "Error: Access Denied. (Path traversal not allowed)\n";
                        send(sd, err_msg, strlen(err_msg), 0);
                        continue;
                    }

                    // 3. Open file (read-only mode O_RDONLY)
                    int file_fd = open(buffer, O_RDONLY);
                    if (file_fd < 0)
                    {
                        char *err_msg = "Error: File Not Found.\n";
                        send(sd, err_msg, strlen(err_msg), 0);
                    }
                    else
                    {
                        // 4. Read file and send to Client
                        char file_buf[1024]; // Buffer for temporary file contents
                        int bytes_read;

                        // Loop read until end of file (read returns 0)
                        while ((bytes_read = read(file_fd, file_buf, sizeof(file_buf))) > 0)
                        {
                            send(sd, file_buf, bytes_read, 0);
                        }

                        // 5. Close file
                        close(file_fd);
                        printf("File '%s' served successfully.\n", buffer);
                    }
                }

                activity--; // Decrement activity count since we handled one event
            }
        }
    }

    return 0;
}