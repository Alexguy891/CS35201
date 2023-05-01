#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 750
#define PORT 8080
#define SA struct sockaddr
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


void func(int sockfd)
{
	// timer
	struct timespec start, end;
    double elapsed_time;

	// create buffer
	char buff[MAX];

	// for read and write
	int n, m;

	// for file
	FILE *fp;
	long filelen;

	// open file
	fp = fopen("file.txt", "rb");
	if(fp == NULL) {
		perror("fopen");
		exit(1);
	}

	// get file length
    fseek(fp, 0, SEEK_END);
    filelen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

	// start the timer
    clock_gettime(CLOCK_MONOTONIC, &start);

	// repeat until end of file
	while (filelen > 0) {
		// read file
		n = fread(buff, sizeof(char), MAX, fp);

		// count bytes sent
		int bytes_sent = 0;

		// repeat while not all bytes sent
		while (bytes_sent < n) {
			// send bytes to server
			int ret = write(sockfd, buff + bytes_sent, n - bytes_sent);
			
			// check for error
			if (ret == -1) {
				perror("write");
				exit(1);
			}
			
			// count bytes sent
			bytes_sent += ret;
		}

		// decrease file length
		filelen -= bytes_sent;

		// report bytes sent to server
		printf("Sent %d bytes to server\n", bytes_sent);

		// wait to receive same number of bytes back from the server
		int bytes_received = 0;

		// repeat while not all bytes received
		while (bytes_received < n) {
			// receive bytes from server
			int ret = read(sockfd, buff + bytes_received, n - bytes_received);

			// check for error
			if (ret == -1) {
				perror("read");
				exit(1);
			}

			// count bytes received
			bytes_received += ret;
		}

		// report bytes received from server
		printf("Received %d bytes from server\n", bytes_received);
	}

	// stop the timer
	clock_gettime(CLOCK_MONOTONIC, &end);


	// calculate time
	elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsed_time += (end.tv_nsec - start.tv_nsec) / 1000000.0;

	// open file
	FILE *fp2;
	fp2 = fopen("results.txt", "a");
	if (fp2 == NULL) {
		perror("Error opening file");
		exit(1);
	}

	// print results
	fprintf(fp2, "Elapsed time: %.2f ms, Sent bytes: ", elapsed_time);
	fprintf(fp2, "%d\n", MAX);
	fclose(fp2);
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}

