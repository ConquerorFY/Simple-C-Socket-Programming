//////////////////////////  Server.c ////////////////
// use this command "gcc hello.c -o hello.o -lws2_32" to compile into exe file (in case encouter error in compilation)
// to use Windows telnet, need to enable the "Telnet Client" feature first, then use command "telnet <IP Addr> <Port>"

#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <ctype.h> // Include ctype.h for toupper function

#define MY_PORT 8989
#define MAXBUF 256

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET sockfd, clientfd;
	struct sockaddr_in self;
	char buffer[MAXBUF];

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
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
	/* bzero(&self, sizeof(self));*/
	self.sin_family = AF_INET;
	self.sin_port = htons(MY_PORT); // Host to Network Short (16-bit)
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

	/*---forever... ---*/
	while (1)
	{
		struct sockaddr_in client_addr;
		int addrlen = sizeof(client_addr);

		/*---accept a connection (creating a data pipe)---*/
		clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
		int recv_size = recv(clientfd, buffer, MAXBUF, 0);

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

		send(clientfd, buffer, recv_size, 0);

		/*---close connection---*/
		close(clientfd);
	}

	/*---clean up (should never get here!)---*/
	close(sockfd);
	WSACleanup();
	return 0;
}
