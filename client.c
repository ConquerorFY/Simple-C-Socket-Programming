#include <stdio.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1" // Replace with the actual IP address
#define SERVER_PORT 8989
#define MAXBUF 256

int main()
{
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAXBUF];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create a socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Failed to create socket. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Failed to connect to server. Error Code: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    while (1)
    {

        // Send a message to the server
        printf("Enter a message to send to the server: ");
        fgets(buffer, MAXBUF, stdin);

        // // Task 3 - Check if the client input is "Exit Client" and terminate if so
        if (strncmp(buffer, "Exit Client", 11) == 0)
        {
            printf("Client requested to exit client. Closing connection.\n");
            exit(0);
        }

        send(clientSocket, buffer, strlen(buffer), 0);

        // Receive the response from the server
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, MAXBUF, 0);

        // Display the received message
        printf("Received from server: %s\n", buffer);
    }

    // Close the socket
    closesocket(clientSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
