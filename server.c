#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 5678
#define BUFSIZE 1024
#define TIMEOUT_SEC 5

// Function to handle a single client connection
void HandleClient(SOCKET ClientSocket) {
    char bufferA[BUFSIZE] = {0};
    char bufferB[BUFSIZE] = {0};
    char bufferID[BUFSIZE] = {0};
    char responseC[BUFSIZE * 3 + 10] = {0}; // Large enough for A + B + ID + formatting
    int recv_result;
    
    printf("[*] Client connected. Waiting for A and B.\n");

    // --- 1. Receive string (A) ---
    recv_result = recv(ClientSocket, bufferA, BUFSIZE - 1, 0);
    if (recv_result > 0) {
        bufferA[recv_result] = '\0';
        printf("    -> Received A: \"%s\"\n", bufferA);
    } else {
        printf("    -> Failed to receive A or client closed connection.\n");
        goto cleanup;
    }
    
    // --- 2. Receive string (B) ---
    recv_result = recv(ClientSocket, bufferB, BUFSIZE - 1, 0);
    if (recv_result > 0) {
        bufferB[recv_result] = '\0';
        printf("    -> Received B: \"%s\"\n", bufferB);
    } else {
        printf("    -> Failed to receive B or client closed connection.\n");
        goto cleanup;
    }
    
    // --- 3. Set a 5-second timeout for the Student ID ---
    
    // Set the receive timeout option
    DWORD timeout = TIMEOUT_SEC * 1000; // Time in milliseconds
    if (setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        printf("    -> ERROR: setsockopt failed with error code %d\n", WSAGetLastError());
        // Do not fail the whole server, but proceed without timeout check
    } else {
        printf("    -> Timeout set for %d seconds. Waiting for Student ID...\n", TIMEOUT_SEC);
    }

    // --- 4. Attempt to receive Student ID ---
    recv_result = recv(ClientSocket, bufferID, BUFSIZE - 1, 0);
    
    if (recv_result > 0) {
        // ID received successfully
        bufferID[recv_result] = '\0';
        printf("    -> Received ID: \"%s\"\n", bufferID);

        // Form string (C) => "Hello World: [ M1234567 ]"
        snprintf(responseC, sizeof(responseC), "%s %s: [ %s ]", bufferA, bufferB, bufferID);
        
    } else if (recv_result == 0) {
        // Connection gracefully closed (unlikely but possible)
        printf("    -> Client closed connection while waiting for ID.\n");
        strcpy(responseC, "Didn't receive student id");

    } else if (WSAGetLastError() == WSAETIMEDOUT) {
        // TIMEOUT occurred
        printf("    -> TIMEOUT: Student ID not received within %d seconds.\n", TIMEOUT_SEC);
        strcpy(responseC, "Didn't receive student id");
        
    } else {
        // Other error (e.g., connection reset)
        printf("    -> Error during ID receive: %d\n", WSAGetLastError());
        strcpy(responseC, "Didn't receive student id");
    }

    // --- 5. Send back string (C) or timeout message ---
    printf("    -> Sending response C: \"%s\"\n", responseC);
    send(ClientSocket, responseC, strlen(responseC), 0);
    
cleanup:
    // Close the client socket
    closesocket(ClientSocket);
    printf("[*] Connection closed.\n");
}


int main() {
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a SOCKET for the server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Setup the TCP listening socket address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Listen on loopback
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(ListenSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("[*] TCP Server running on 127.0.0.1:%d. Waiting for connections...\n", PORT);

    // Main server loop
    while (1) {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            continue; // Continue listening for the next connection
        }
        
        // Handle the client (Note: for simplicity, this example is single-threaded)
        HandleClient(ClientSocket);
    }
    
    // Cleanup (unreachable in infinite loop but good practice)
    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}
