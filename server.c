#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in structure
#include <string.h>     // For bzero
#include <stdio.h>      // For printf, scanf, perror
#include <arpa/inet.h>  // For inet_addr
#include <unistd.h>     // For close

// Define the port and IP address
#define PORT 5678
#define IP_ADDRESS "127.0.0.1"

int main() {
    // Structure for server and client address information
    struct sockaddr_in server, client;
    int sock, addressSize;
    int num1, num2, ans; // Variables to store numbers and answer

    // 1. Initialize server structure
    bzero(&server, sizeof(server));
    server.sin_family = PF_INET;
    // Set server IP address (localhost)
    server.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    // Set server port, converted to network byte order
    server.sin_port = htons(PORT);

    // 2. Create a UDP socket (SOCK_DGRAM)
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return 1;
    }

    // 3. Bind the socket to the server address and port
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        close(sock);
        return 1;
    }

    printf("UDP Server is running on %s:%d. Waiting for data...\n", IP_ADDRESS, PORT);

    addressSize = sizeof(client);

    // 4. Receive the first number (num1)
    // The client address is saved in the 'client' struct
    if (recvfrom(sock, &num1, sizeof(num1), 0, (struct sockaddr *)&client, &addressSize) < 0) {
        perror("recvfrom num1 failed");
        close(sock);
        return 1;
    }
    printf("Read Message [1]: %d\n", num1);

    // 5. Receive the second number (num2)
    // Note: The client's address (client struct) remains the same
    if (recvfrom(sock, &num2, sizeof(num2), 0, (struct sockaddr *)&client, &addressSize) < 0) {
        perror("recvfrom num2 failed");
        close(sock);
        return 1;
    }
    printf("Read Message [2]: %d\n", num2);

    // 6. Perform calculation
    ans = num1 + num2;
    printf("Answer: %d\n", ans);

    // 7. Send the result back to the client
    if (sendto(sock, &ans, sizeof(ans), 0, (struct sockaddr *)&client, sizeof(client)) < 0) {
        perror("sendto answer failed");
        close(sock);
        return 1;
    }

    // 8. Close the socket
    close(sock);
    return 0;
}
