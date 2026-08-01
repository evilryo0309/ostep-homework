#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30
#define CACHE_SIZE 5

#define STATE_UNUSED 0
#define STATE_IDLE 1
#define STATE_READING 2

// --- 1. User-Level Cache Structure Design ---
typedef struct
{
    char filename[256];
    char content[4096];
    int size;
    int is_valid;
} CacheEntry;

CacheEntry cache[CACHE_SIZE];

// Global Flag: indicates whether a signal to clear the cache was received
// volatile ensures compiler won't optimize it into a register, sig_atomic_t guarantees read/write atomicity
volatile sig_atomic_t clear_cache_flag = 0;

// --- 2. Signal Handler Implementation ---
// Keep it minimal, only set Flag, do not call printf or perform complex operations inside
void handle_sighup(int sig)
{
    clear_cache_flag = 1;
}

// Helper function: Clear Cache
void flush_cache()
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        cache[i].is_valid = 0;
        cache[i].filename[0] = '\0';
    }
    printf("\n[Signal Handler] SIGHUP received. Cache successfully cleared!\n");
}

// Helper function: Search Cache
int find_in_cache(const char *filename)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (cache[i].is_valid && strcmp(cache[i].filename, filename) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Helper function: Write to Cache (simple replacement strategy)
void add_to_cache(const char *filename, const char *content, int size)
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (!cache[i].is_valid)
        {
            strncpy(cache[i].filename, filename, sizeof(cache[i].filename) - 1);
            memcpy(cache[i].content, content, size);
            cache[i].size = size;
            cache[i].is_valid = 1;
            printf("[Cache] Stored '%s' into cache slot %d.\n", filename, i);
            return;
        }
    }
    // If full, overwrite slot 0 (simplified demonstration)
    strncpy(cache[0].filename, filename, sizeof(cache[0].filename) - 1);
    memcpy(cache[0].content, content, size);
    cache[0].size = size;
    cache[0].is_valid = 1;
    printf("[Cache] Cache full. Overwrote '%s' into cache slot 0.\n", filename);
}

// --- 3. Main Program Structure ---
struct client_context
{
    int state;
    int socket_fd;
    int file_fd;
    off_t file_offset;
    struct aiocb cb;
    char buffer[BUFFER_SIZE];
    char filename[256];
};

int main()
{
    int server_fd, new_socket, max_sd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set readfds;

    // Register SIGHUP signal
    struct sigaction sa;
    sa.sa_handler = handle_sighup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    // Initialize Cache
    flush_cache();
    clear_cache_flag = 0; // Reset Flag

    struct client_context clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].state = STATE_UNUSED;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server running (PID: %d). Listening on port %d...\n", getpid(), PORT);
    printf("Send SIGHUP using: kill -SIGHUP %d\n", getpid());

    while (1)
    {
        // --- Check Signal Flag ---
        if (clear_cache_flag)
        {
            flush_cache();
            clear_cache_flag = 0; // Reset Flag after clearing
        }

        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].state == STATE_IDLE)
            {
                FD_SET(clients[i].socket_fd, &readfds);
                if (clients[i].socket_fd > max_sd)
                    max_sd = clients[i].socket_fd;
            }
        }

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            if (errno == EINTR)
            {
                // Interrupted by signal! Return directly to top of while(1) loop to check Signal Flag
                continue;
            }
            else
            {
                perror("select error");
                continue; // Return to top for other errors to avoid handling invalid readfds
            }
        }

        // --- Event A: Handle Network Events ---
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
            {
                perror("accept error");
            }
            else
            {
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
                    clients[i].buffer[valread] = '\0';
                    clients[i].buffer[strcspn(clients[i].buffer, "\r\n")] = 0;
                    strncpy(clients[i].filename, clients[i].buffer, sizeof(clients[i].filename) - 1);

                    if (strstr(clients[i].filename, "..") != NULL || clients[i].filename[0] == '/')
                    {
                        char *err_msg = "Error: Access Denied.\n";
                        send(clients[i].socket_fd, err_msg, strlen(err_msg), 0);
                        continue;
                    }

                    // Check Cache Hit
                    int cache_idx = find_in_cache(clients[i].filename);
                    if (cache_idx != -1)
                    {
                        printf("[Cache HIT] Serving '%s' directly from memory!\n", clients[i].filename);
                        send(clients[i].socket_fd, cache[cache_idx].content, cache[cache_idx].size, 0);
                        close(clients[i].socket_fd);
                        clients[i].state = STATE_UNUSED;
                        continue;
                    }

                    printf("[Cache MISS] Opening file '%s' from disk...\n", clients[i].filename);
                    clients[i].file_fd = open(clients[i].filename, O_RDONLY);
                    if (clients[i].file_fd < 0)
                    {
                        char *err_msg = "Error: File Not Found.\n";
                        send(clients[i].socket_fd, err_msg, strlen(err_msg), 0);
                    }
                    else
                    {
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
                            clients[i].state = STATE_READING;
                        }
                    }
                }
            }
        }

        // --- Event B: Poll AIO Status ---
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].state == STATE_READING)
            {
                int err = aio_error(&clients[i].cb);

                if (err == 0)
                {
                    int bytes_read = aio_return(&clients[i].cb);

                    if (bytes_read > 0)
                    {
                        send(clients[i].socket_fd, clients[i].buffer, bytes_read, 0);

                        // Write result to Cache (demonstration of small file caching)
                        add_to_cache(clients[i].filename, clients[i].buffer, bytes_read);

                        clients[i].file_offset += bytes_read;
                        clients[i].cb.aio_offset = clients[i].file_offset;

                        if (aio_read(&clients[i].cb) == -1)
                        {
                            perror("aio_read failed on continue");
                        }
                    }
                    else
                    {
                        close(clients[i].file_fd);
                        close(clients[i].socket_fd);
                        clients[i].state = STATE_UNUSED;
                    }
                }
                else if (err != EINPROGRESS)
                {
                    close(clients[i].file_fd);
                    close(clients[i].socket_fd);
                    clients[i].state = STATE_UNUSED;
                }
            }
        }
    }

    return 0;
}