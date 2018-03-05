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

// struct ircmsg {
// 	char prefix[BUF_SIZE];
// 	char command[CMD_MAXLEN];
// 	char params[15][BUF_SIZE];
// }

/* macro so static array sizes can be set */
#define BUF_SIZE 513
static const int CMD_MAXLEN = 16;
static const int MAX_PARAMS = 15;
static const char* SERVER = "irc.freenode.net";
static const int PORT = 6667;
// static const char* CHANNEL = "##bottesting19283746";
static const char* NICKNAME = "Gertrudes";
static const char* USERNAME = "bottybot";
// static const char* HOSTNAME = "host?";
// static const char* SERVERNAME = "server?";
static const char* REALNAME = "Not a robot";

static int sockfd;
// TODO: maybe put these inside ircread as static variables?
static char ircbuf[BUF_SIZE*2];
static int ircbuf_len;

/***** user commands *****/
/* exits the program */
static const char* COMMAND_EXIT = "!exit";

/***** irc commands *****/
static const char* IRC_PING = "PING";

/***** error codes *****/
/* bad socket (e.g. not connected, not valid socket FD) */
static const int ERR_NOSOCKET = -2;
static const int ERR_SOCKETWRITE = -3;
static const int ERR_BADREAD = -4; /* read() */
// static const int ERR_BADWRITE = -5; /* write() */
static const int ERR_BADGET = -6; /* fgets() */
// static const int ERR_BADPUT = -7; /* fputs() */
// static const int ERR_BADMESSAGE = -8; /* error parsing an IRC message */

void circus();
int ircconnect();
/* Returns -1 on error, 0 on success, 1 on !exit command */
int user_read();
int ircread();
/* Closes sockfd and sets its value to -1 */
void ircclosesocket();
int handlemessage(const char* message);

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

	debug_print("server->h_name: %s\n", server->h_name);

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

	ircbuf[0] = '\0';
	ircbuf_len = 0;
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

		debug_print("test\n", NULL);
		ready = select(maxfd+1, &rd_set, &wr_set, &er_set, NULL);

		if (ready) {
			debug_print("ready: %d\n", ready);
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

	debug_print("user - %d: %s\n", sockfd, buffer);

	trim_newline(buffer);

	if (strcmp(buffer, COMMAND_EXIT) == 0)
		return 1;

	return 0;
}

/*
   ircread uses the persistent buffer ircbuf to store what is read from
   the socket across calls. Any time a newline is found, the function
   extracts that message from the buffer and calls handlemessage() with
   it. It does so until there are no more new lines in the permanent
   buffer.
*/
int ircread() {
	int n;
	char buffer[BUF_SIZE];
	char message[BUF_SIZE];
	char* crlf;

	if (sockfd < 0)
		return ERR_NOSOCKET;

	memset(buffer, 0, sizeof(buffer));

	debug_print("server - %d: ", sockfd);

	if ((n = read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
		buffer[n] = '\0';
		debug_print("\n\tircbuf_len: %d\n\n\tn: %d\n", ircbuf_len, n);
		strncpy(ircbuf+ircbuf_len, buffer, n+1); /* n+1 to capture the '\0' */
		ircbuf_len += n;

		debug_print("\n\tbuffer: %s|||||\n", buffer);
		debug_print("\n\tircbuf: %s|||||\n", ircbuf);

		/* we have a message! */
		while ((crlf = strpbrk(ircbuf, "\r\n")) != NULL) {
			int index = crlf - ircbuf;
			int status;
		
			debug_print("\n\tindex: %d\n", index);

			strncpy(message, ircbuf, index);
			message[index] = '\0';

			ircbuf_len -= index;
			memmove(ircbuf, ircbuf+index, ircbuf_len+1); /* ircbuf_len+1 to capture the '\0' */
			ircbuf_len -= trim_left(ircbuf);
			ircbuf[ircbuf_len]='\0';

			debug_print("\n-----> passing message: ||%s||\n", message);
			if ((status = handlemessage(message)) < 0)
				return status;
		}
	}
	else {
		ircclosesocket();
		if (n == 0) {
			/* socket closed on other end, close here and try reconnecting */
			return ERR_NOSOCKET;
		}
		else { /* n < 0 */
			/* error reading from socket, close socket and try reconnecting */
			return ERR_BADREAD;
		}
	}

	return 0;
}

int ircwrite(char* s) {
	printf("WILL WRITE THIS TO SERVER: %s\n", s);
	
	// if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("writing to socket");

	// memset(buffer, 0, sizeof(buffer));
	// if ((n = read(sockfd, buffer, strlen(buffer))) < 0)
	// 	error("reading from socket");

	
	return 0;
}

void ircclosesocket() {
	close(sockfd);
	sockfd = -1;
}

int handlemessage(const char* message) {
	int i;
	char msg[BUF_SIZE]; /* mutable copy of message */
	char* tok;
	char command[CMD_MAXLEN];
	char params[MAX_PARAMS][BUF_SIZE];
	char response[BUF_SIZE];

	/* initialize strings */
	strncpy(msg, message, BUF_SIZE-1);
	msg[BUF_SIZE-1] = '\0'; // TODO: sizeof in all these vs just using each macro?
	command[0] = '\0';
	response[0] = '\0';
	for (i = 0; i < MAX_PARAMS; i++)
		params[i][0] = '\0';

	// TODO: should I be using strtok_r() here, or is strtok() enough?
	tok = strtok(msg, " ");
	/* if the message starts with a :, first part is the prefix */
	if (msg[0] == ':')
		tok = strtok(NULL, " ");
	if (tok == NULL){
		/* not a message */
		// TODO: figure out if errors can actually be seen here and how to handle them
		printf("[?] Not a message: ");
		goto print;
	}
	strncpy(command, tok, sizeof(command)-1);
	command[CMD_MAXLEN-1] = '\0';
	
	/* get params */
	i = 0;
	while ((tok = strtok(NULL, " ")) != NULL && (i <= MAX_PARAMS)) {
		debug_print("tok: |%s|\n", tok);
		/* if the param starts with a :, it is the last param and can include spaces */
		if (tok[0] ==':') {
			int toklen;
			char* rest;

			debug_print("i: %d\n", i);
			toklen = strlen(tok+1);
			rest = strtok(NULL, "");

			/* copy the token that includes the : (without the :), */
			debug_print("toklen: %d  rest: %s\n", toklen, rest);
			strncpy(params[i], tok+1, BUF_SIZE-1);
			debug_print("params[%d]: %s\n", i, params[i]);

			/* then, if there is more, add a space and copy the rest of the msg */
			if (rest != NULL) {
				debug_print("here %d\n", i);
				params[i][toklen] = ' ';
				strncpy(params[i]+toklen+1, rest, BUF_SIZE-toklen-1);
				params[i][BUF_SIZE-1] = '\0';
				debug_print("here %d\n", i);
			}

			break;
		}
		debug_print("here %d\n", i);
		strncpy(params[i], tok, BUF_SIZE-1);
		params[i][BUF_SIZE-1] = '\0';
		i++;
	}
	
	/* command responses */
	if (strcmp(command, IRC_PING) == 0) {
		// TODO: handle errors
		sprintf(response, "PONG %s\r\n", params[0]);
		ircwrite(response);
	}
	else
print:
		printf("HANDLING MESSAGE: %s\n", message);
		printf("\t COMMAND: %s\n\t PARAMS:\n", command);
		for (i = 0; i < MAX_PARAMS; i++) {
			if (strlen(params[i]) > 0)
				printf("\t\t [%2d]: %s\n", i, params[i]);
		}

	return 0;
}
