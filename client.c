#include <sys/socket.h>  // For socket functions
#include <netinet/in.h>  // For sockaddr_in structure
#include <sys/select.h>  // For select() and FD_ macros
#include <sys/time.h>    // For timeval
#include <string.h>      // For bzero
#include <stdio.h>       // For printf, scanf, perror
#include <arpa/inet.h>   // For inet_addr
#include <unistd.h>      // For close

// Define the port and IP address
#define PORT 5678
#define IP_ADDRESS "127.0.0.1"
#define TIMEOUT_SEC 3 // Timeout for user input

int main() {
    // File descriptor set for select (we will monitor standard input, fd 0)
    fd_set rfds;
    struct sockaddr_in server;
    struct timeval tv; // Time-out structure
    
    int sock, readSize, addressSize;
    int retval; // Return value for select
    int num1, num2, ans; // Variables for numbers and answer
    
    // 1. Initialize server structure
    bzero(&server, sizeof(server));
    server.sin_family = PF_INET;
    // Set server IP address (localhost)
    server.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    // Set server port
    server.sin_port = htons(PORT);

    // 2. Create a UDP socket (SOCK_DGRAM)
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return 1;
    }

    addressSize = sizeof(server);

    // 3. Get the first number from the user
    printf("Enter the first number (num1): ");
    if (scanf("%d", &num1) != 1) {
        fprintf(stderr, "Invalid input for num1.\n");
        close(sock);
        return 1;
    }

    // 4. Send the first number to the server
    if (sendto(sock, &num1, sizeof(num1), 0, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("sendto num1 failed");
        close(sock);
        return 1;
    }

    printf("Sent num1: %d. Now waiting up to %d seconds for the second number...\n", num1, TIMEOUT_SEC);
    
    // 5. Setup file descriptor set and timeout for select()
    FD_ZERO(&rfds);   // Clear the set
    FD_SET(0, &rfds); // Add standard input (file descriptor 0) to the set

    // Set the timeout value to 5 seconds
    tv.tv_sec = TIMEOUT_SEC; 
    tv.tv_usec = 0; 
    
    // 6. Use select() to wait for data on standard input (fd 0)
    // The first argument (n) for select is the highest-numbered file descriptor plus one.
    // Here, we monitor fd 0 (stdin), so n=1 is correct.
    retval = select(1, &rfds, NULL, NULL, &tv);

    if (retval == -1) {
        // select error
        perror("select error!");
        num2 = 0; // Default error value
    } else if (retval) {
        // Data is available (user input detected)
        printf("Input detected. Enter the second number (num2): ");
        if (scanf("%d", &num2) != 1) {
            fprintf(stderr, "Invalid input for num2, using default (0).\n");
            num2 = 0;
        }
    } else {
        // Timeout occurred (retval is 0)
        num2 = 100; // Use the default value 100 as per your original code
        printf("Timeout reached. Sending default num2: %d.\n", num2);
    }
    
    // 7. Send the second number (either user input or default 100)
    if (sendto(sock, &num2, sizeof(num2), 0, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("sendto num2 failed");
        close(sock);
        return 1;
    }

    // 8. Receive the final result from the server
    printf("Waiting for server's answer...\n");
    readSize = recvfrom(sock, &ans, sizeof(ans), 0, (struct sockaddr *)&server, &addressSize);
    
    if (readSize < 0) {
        perror("recvfrom answer failed");
        close(sock);
        return 1;
    }
    
    printf("Read Message (Answer): %d\n", ans);

    // 9. Close the socket
    close(sock);
    return 0;
}
