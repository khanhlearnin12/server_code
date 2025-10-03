#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 5678
#define BUFSIZE 1024

// Helper function to read a line from stdin
int read_input(const char* prompt, char* buffer, size_t max_len) {
    printf("%s", prompt);
    if (fgets(buffer, max_len, stdin) == NULL) {
        // EOF (CTRL+Z on Windows console) encountered
        return -1;
    }
    // Remove the trailing newline character, if present
    buffer[strcspn(buffer, "\n")] = 0;
    return 0;
}

// Validation function for String A
// Character count must be between 5 to 10
int is_A_legal(const char* s) {
    size_t len = strlen(s);
    return (len >= 5 && len <= 10);
}

// Validation function for String B
// Character count must be even
int is_B_legal(const char* s) {
    size_t len = strlen(s);
    return (len > 0 && (len % 2 == 0));
}

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    char bufferA[BUFSIZE];
    char bufferB[BUFSIZE];
    char bufferID[BUFSIZE];
    char recvbuf[BUFSIZE];
    int bytes_received;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Main client loop (runs endlessly until EOF)
    while (1) {
        // Reset buffers for the new round
        memset(bufferA, 0, BUFSIZE);
        memset(bufferB, 0, BUFSIZE);
        memset(bufferID, 0, BUFSIZE);

        printf("\n-------------------------------------------------\n");
        printf("Please enter three inputs (CTRL+Z then ENTER to quit):\n");

        // 1. Take input for String A
        if (read_input("String A (5-10 chars): ", bufferA, BUFSIZE) == -1) break;
        
        // 1. Take input for String B
        if (read_input("String B (Even length): ", bufferB, BUFSIZE) == -1) break;
        
        // 1. Take input for Student ID
        if (read_input("Student ID: ", bufferID, BUFSIZE) == -1) break;
        
        // 4. Make sure the string (A) and (B) are legal
        if (!is_A_legal(bufferA) || !is_B_legal(bufferB)) {
            printf("\n[ERROR] Validation failed: String A (5-10 chars) or String B (even length) is illegal.\n");
            printf("error\n");
            continue; // Go to the next iteration
        }
        
        printf("[INFO] Inputs are legal. Connecting to server...\n");

        // Create a SOCKET for connection
        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("[ERROR] socket failed with error: %d\n", WSAGetLastError());
            // Need to cleanup Winsock but also continue the loop gracefully if possible
            continue;
        }

        // Setup the TCP server address
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_addr.sin_port = htons(PORT);
        
        // Connect to server
        if (connect(ConnectSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            printf("[ERROR] Connect failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            continue;
        }
        
        printf("[INFO] Connected. Sending A, B, and ID sequentially.\n");

        // 2. Send A, B, and ID to the TCP echo server
        send(ConnectSocket, bufferA, (int)strlen(bufferA), 0);
        send(ConnectSocket, bufferB, (int)strlen(bufferB), 0);
        send(ConnectSocket, bufferID, (int)strlen(bufferID), 0);

        // 3. Receive the echo reply string and print it
        bytes_received = recv(ConnectSocket, recvbuf, BUFSIZE - 1, 0);
        if (bytes_received > 0) {
            recvbuf[bytes_received] = '\0';
            printf("\n<<< SERVER REPLY >>>\n%s\n", recvbuf);
        } else if (bytes_received == 0) {
            printf("[INFO] Connection closed by server.\n");
        } else {
            printf("[ERROR] Receive failed with error: %d\n", WSAGetLastError());
        }

        // Close socket and start over (necessary because the server closes the socket after each response)
        closesocket(ConnectSocket);
    }
    
    printf("\n[INFO] EOF entered. Closing connection and exiting.\n");
    // Final cleanup
    WSACleanup();
    return 0;
}
