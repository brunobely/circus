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

static int sockfd;

/***** user commands *****/
/* exits the program */
static const char* COMMAND_EXIT = "!exit";

/***** error codes *****/
/* bad socket (e.g. not connected, not valid socket FD) */
static const int ERR_NOSOCKET = -2;
static const int ERR_SOCKETWRITE = -3;
static const int ERR_BADREAD = -4; /* read() */
// static const int ERR_BADWRITE = -5; /* write() */
static const int ERR_BADGET = -6; /* fgets() */
static const int ERR_BADPUT = -7; /* fputs() */

void circus();
int ircconnect();
/* Returns -1 on error, 0 on success, 1 on !exit command */
int user_read();
int ircread();
/* Closes sockfd and sets its value to -1 */
void ircclosesocket();
void trim_newline(char *s);

// TODO: clean this up and refactor
void circus() {
	fd_set rd_set, wr_set, er_set; /* fd sets for select */

	struct sockaddr_in serv_addr;
	struct hostent* server;

	// char buffer[BUF_SIZE];

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

	if (ircconnect() < 0)
		error("ircconnect");
		

	while (1) {
		int stdinfd = fileno(stdin);
		int maxfd = stdinfd > sockfd ? stdinfd : sockfd;
		int ready;
		int status = 0;

		if (sockfd < 0) {
			// TODO: try reconnecting
			printf("No socket, exiting.\n");
			exit(1);
		}

		/* empty the fd sets */
		FD_ZERO(&rd_set);
		// FD_ZERO(&wr_set);
		// FD_ZERO(&er_set);
		/* select on read, write, and error for the socket or stdin */
		FD_SET(sockfd, &rd_set);
		// FD_SET(sockfd, &wr_set);
		// FD_SET(sockfd, &er_set);
		FD_SET(stdinfd, &rd_set);
		// FD_SET(stdinfd, &wr_set);
		// FD_SET(stdinfd, &er_set);

		ready = select(maxfd+1, &rd_set, &wr_set, &er_set, NULL);

		if (ready) {
			printf("ready: %d\n", ready);
			if (FD_ISSET(stdinfd, &rd_set)) {
				// /* if user command is !exit or EOF */
				if ((status = user_read()) < 0)
					fprintf(stderr, "error: user_read\n");
				else if (status > 0)
					break;
			}
			if (FD_ISSET(sockfd, &rd_set)) {
				if ((status = ircread()) < 0)
					fprintf(stderr, "error: ircread\n");
				if (status == ERR_NOSOCKET) {
					// TODO: try reconnecting
					printf("No socket, exiting.\n");
					exit(1);
				}
			}
		}
		else {
			perror("select");
		}
	}
}

int ircconnect() {
	char buffer[BUF_SIZE];
	sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	if (write(sockfd, buffer, strlen(buffer)) < 0)
		return ERR_SOCKETWRITE;

	return 0;
}

int user_read() {
	char buffer[BUF_SIZE];
	memset(buffer, 0, sizeof(buffer));
	if (fgets(buffer, sizeof(buffer), stdin) == NULL)
		return ERR_BADGET;

	// if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("writing to socket");

	// memset(buffer, 0, sizeof(buffer));
	// if ((n = read(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("reading from socket");

	printf("user - %d: %s\n", sockfd, buffer);

	trim_newline(buffer);

	if (strcmp(buffer, COMMAND_EXIT) == 0)
		return 1;

	return 0;
}

int ircread() {
	int n;
	char buffer[BUF_SIZE];

	if (sockfd < 0)
		return ERR_NOSOCKET;

	memset(buffer, 0, sizeof(buffer));

	printf("server - %d: ", sockfd);

	if ((n = read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
		buffer[n] = '\0';
		// TODO: don't use fputs as it inserts a newline at the end
		if (fputs(buffer, stdout) == EOF)
			return ERR_BADPUT;
	}
	else if (n == 0) {
		/* socket closed on other end, close here and try reconnecting */
		ircclosesocket();
	}
	else { /* n < 0 */
		/* error reading from socket, close socket and try reconnecting */
		ircclosesocket();

		printf("\n");

		return ERR_BADREAD;
	}

	return 0;
}

void ircclosesocket() {
	close(sockfd);
	sockfd = -1;
}

/* s: NUL-terminated string */
void trim_newline(char *s) {
	int l = strlen(s);
	if (s[l-1] == '\n')
		s[l-1] = '\0';
}
