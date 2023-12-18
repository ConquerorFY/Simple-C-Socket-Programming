#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <ctype.h>
#include <windows.h>
#include <process.h> // For _beginthreadex
#include <time.h>
#include <string.h>

#define MY_PORT 8989
#define MAXBUF 256

// Function to handle client requests in a separate thread
DWORD WINAPI HandleClient(void *clientSocket)
{
    SOCKET clientfd = *((SOCKET *)clientSocket);
    char buffer[MAXBUF];
    while (1)
    {
        memset(buffer, '\0', sizeof(buffer));
        int recv_size = recv(clientfd, buffer, MAXBUF, 0);

        // Task 2 - Display the length of the received message
        printf("Received message length: %d\n", recv_size);

        if (recv_size < 0)
            break;

        // Task 2 - Check if the client input is "Exit Server"
        if (strncmp(buffer, "Exit Server", 11) == 0)
        {
            printf("Client requested to exit the server. Closing connection.\n");
            exit(0);
        }

        // Task 4 - Check if "Date" command is being sent by client, return current date time if so
        if (strncmp(buffer, "Date", 4) == 0)
        {
            time_t current_time;
            struct tm *time_info;

            // Get current time
            time(&current_time);
            time_info = localtime(&current_time);

            // Format time string
            // strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\r\n", time_info);
            strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y %Z\r\n", time_info);

            // Send the date time string back to the client
            send(clientfd, buffer, sizeof(buffer), 0);

            continue;
        }

        // Task 5 - Check if "Time XXX" command is being sent, return current time in the current time zone (with XXX being time zone)
        if (strstr(buffer, "Time ") != NULL)
        {
            char timezone[MAXBUF];
            strncpy(timezone, buffer + 5, 4);

            if (strlen(buffer) == 9)
            {
                timezone[3] = '\0';
            }
            else if (strlen(buffer) == 10)
            {
                timezone[4] = '\0';
            }

            time_t now;
            struct tm *time_info;

            // Get current time
            time(&now);
            time_info = localtime(&now);

            time_t rawTime;

            // Convert struct tm to time_t
            rawTime = mktime(time_info);

            int hoursToAdd;
            if (strcmp(timezone, "PST") == 0)
            {
                hoursToAdd = -8;
            }
            else if (strcmp(timezone, "MST") == 0)
            {
                hoursToAdd = -7;
            }
            else if (strcmp(timezone, "CST") == 0)
            {
                hoursToAdd = -6;
            }
            else if (strcmp(timezone, "EST") == 0)
            {
                hoursToAdd = -5;
            }
            else if (strcmp(timezone, "GMT") == 0)
            {
                hoursToAdd = 0;
            }
            else if (strcmp(timezone, "CET") == 0)
            {
                hoursToAdd = 1;
            }
            else if (strcmp(timezone, "MSK") == 0)
            {
                hoursToAdd = 3;
            }
            else if (strcmp(timezone, "JST") == 0)
            {
                hoursToAdd = 9;
            }
            else if (strcmp(timezone, "AEDT") == 0)
            {
                hoursToAdd = 11;
            }
            else
            {
                strcpy(buffer, "ERROR");
                // Send the time string back to the client
                send(clientfd, buffer, sizeof(buffer), 0);
                continue;
            }

            // Add hours
            rawTime += hoursToAdd * 3600;

            // Convert time_t back to struct tm
            *time_info = *localtime(&rawTime);

            // Format time string
            // strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S\r\n", time_info);
            strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y %Z\r\n", time_info);

            // Send the time string back to the client
            send(clientfd, buffer, sizeof(buffer), 0);

            continue;
        }

        // Task 1 - Convert all alphabets to uppercase and reverse message
        for (int i = 0; i < recv_size; i++)
        {
            if (isalpha(buffer[i]))
            {
                buffer[i] = toupper(buffer[i]);
            }
        }
        for (int i = 0, j = recv_size - 1; i < j; i++, j--)
        {
            char temp = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = temp;
        }

        // Send the modified message back to the client
        send(clientfd, buffer, recv_size, 0);
    }

    // Close the connection
    closesocket(clientfd);
    return 0;
}

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET sockfd, clientfd;
    struct sockaddr_in self, client_addr;
    char buffer[MAXBUF];

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    /*---create streaming socket---*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket");
        exit(errno);
    }

    printf("Socket created.\n");

    /*---initialize address/port structure---*/
    self.sin_family = AF_INET;
    self.sin_port = htons(MY_PORT);
    self.sin_addr.s_addr = INADDR_ANY;

    /*---assign a port number to the socket---*/
    if (bind(sockfd, (struct sockaddr *)&self, sizeof(self)) != 0)
    {
        perror("socket--bind");
        exit(errno);
    }

    puts("Bind done");

    /*---make it a "listening socket"---*/
    if (listen(sockfd, 20) != 0)
    {
        perror("socket--listen");
        exit(errno);
    }

    puts("Waiting for incoming connections...");

    while (1)
    {
        int addrlen = sizeof(client_addr);

        /*---accept a connection (creating a data pipe)---*/
        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);

        // Task 2 - Display client information
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Task 2 - Create threads to handle the client requests concurrently
        HANDLE hThread = CreateThread(NULL, 0, HandleClient, &clientfd, 0, NULL);
        if (hThread == NULL)
        {
            fprintf(stderr, "Error creating thread.\n");
            return 1;
        }

        // Close the thread handle to avoid resource leaks
        CloseHandle(hThread);
    }

    /*---clean up---*/
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
