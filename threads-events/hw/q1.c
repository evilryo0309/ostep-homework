#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 256

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    time_t rawtime;
    struct tm *timeinfo;

    // 1. Create TCP Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Configure Server Address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all available network interfaces
    server_addr.sin_port = htons(PORT);       // Set Port and convert to Network Byte Order

    // 3. Bind Socket to specified Port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Start listening for connections (Backlog set to 5)
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Time Server is listening on port %d...\n", PORT);

    // 5. Enter main loop, service one Request at a time
    while (1)
    {
        // accept will block until a Client connects
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0)
        {
            perror("Accept failed");
            continue; // On failure, try to receive the next one
        }

        // Get current time
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Format time as string
        snprintf(buffer, BUFFER_SIZE, "Current Time: %s", asctime(timeinfo));

        // Send to Client
        send(client_fd, buffer, strlen(buffer), 0);
        printf("Served one request.\n");

        // Finished processing, close the Client's Socket
        close(client_fd);
    }

    close(server_fd);
    return 0;
}