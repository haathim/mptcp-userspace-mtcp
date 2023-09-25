#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.60.3"
#define SERVER_PORT 12345

int main() {

    
    int sockfd;
    struct sockaddr_in server_addr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sockfd);
        return 1;
    }

    char message1[] = "Hello, server! This is message 1.";
    char message2[] = "Hi again, server! This is message 2.";
    char buffer[1024];
    ssize_t bytes_received;

    // Send and receive for message 1
    send(sockfd, message1, strlen(message1), 0);
    printf("Sent: %s\n", message1);
    bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
    }

    // Send and receive for message 2
    send(sockfd, message2, strlen(message2), 0);
    printf("Sent: %s\n", message2);
    bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Received: %s\n", buffer);
    }

    // Close the socket
    close(sockfd);

    return 0;
}