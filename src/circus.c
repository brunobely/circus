#include <stdio.h>
#include <unistd.h> /* read, write */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> /* hostent */
#include <string.h> /* memset, memcpy */

#include "circus.h"
// #include "str.h"
#include "util.h"

static const int BUF_SIZE = 512;
static const char* SERVER = "irc.freenode.net";
static const int PORT = 6667;
// static const char* CHANNEL = "##bottesting19283746";
static const char* NICKNAME = "Gertrudes";
static const char* USERNAME = "bottybot";
// static const char* HOSTNAME = "host?";
// static const char* SERVERNAME = "server?";
static const char* REALNAME = "Not a robot";

void circus();
int ircconnect(int sockfd);

// TODO: clean this up and refactor
void circus() {
	int sockfd, n;

	struct sockaddr_in serv_addr;
	struct hostent* server;

	char buffer[BUF_SIZE];

	// TODO: handle errors properly
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("socket");

	if ((server = gethostbyname(SERVER)) == NULL)
		error("gethostbyname");

	printf("server->h_name: %s\n", server->h_name);

	memset((char*) &serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	memcpy((char*) &serv_addr.sin_addr.s_addr,
		(char*) server->h_addr,
		server->h_length);

	serv_addr.sin_port = htons(PORT);

	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		error("connect");

	// printf("Please enter message:\n");
	// memset(buffer, 0, sizeof(buffer));
	// fgets(buffer, 255, stdin);
	
	// if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("writing to socket");

	// memset(buffer, 0, sizeof(buffer));
	// if ((n = read(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("reading from socket");

	// printf("%s\n", buffer);

	if (ircconnect(sockfd) < 0)
		error("ircconnect");

	memset(buffer, 0, sizeof(buffer));
	while ((n = read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
		printf("\nloop\n");
		buffer[n] = '\0';
		if (fputs(buffer, stdout) == EOF)
			error("fputs");
	}
	if (n < 0)
		error("reading from socket");

	printf("final: %s\n", buffer);
}

int ircconnect(int sockfd) {
	char buffer[BUF_SIZE];
	sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	if (write(sockfd, buffer, strlen(buffer)) < 0)
		return -1;

	return 0;
}
