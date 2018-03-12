#include <stdio.h>
#include <unistd.h> /* read, write */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> /* hostent */
#include <string.h> /* memset, memcpy */

#include "constants.h"
#include "circus.h"
#include "irc.h"
// #include "str.h"
#include "error.h"
#include "util.h"
#include "message_q.h"

// struct ircmsg {
// 	char prefix[MSG_MAXSIZE];
// 	char command[CMD_MAXLEN];
// 	char params[15][MSG_MAXSIZE];
// }

// TODO: put these inside sv connection
int sockfd;
char ircbuf[MSG_MAXSIZE*2];
int ircbuf_len;
struct message_q* q;

void circus();
/* Returns -1 on error, 0 on success, CCODE_EXIT on !exit command */
int user_read();
int ircwrite(char* s);
int ircsendmessage(struct irc_message m);
int handlemessage(const char* message);

// TODO: clean this up and refactor
void circus() {
	fd_set rd_set, wr_set, er_set; /* fd sets for select */

	struct sockaddr_in serv_addr;
	struct hostent* server;

	// TODO: free this before exiting?
	q = message_q();
	// char buffer[MSG_MAXSIZE];

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

	if (ircregister(sockfd) < 0)
		error("ircregister");

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
		FD_ZERO(&wr_set);
		// FD_ZERO(&er_set);
		/* select on read, write, and error for the socket or stdin */
		FD_SET(sockfd, &rd_set);
		FD_SET(sockfd, &wr_set);
		// FD_SET(sockfd, &er_set);
		FD_SET(stdinfd, &rd_set);
		// FD_SET(stdinfd, &wr_set);
		// FD_SET(stdinfd, &er_set);

		debug_print("test\n", NULL);
		ready = select(maxfd+1, &rd_set, &wr_set, &er_set, NULL);

		if (ready) {
			debug_print("ready: %d\n", ready);
			if (FD_ISSET(stdinfd, &rd_set)) {
				status = user_read();

				/* if user command is !exit or EOF */
				// TODO: the EOF part
				if (status == CCODE_EXIT)
					// TODO: call ircexit()
					break;
				// TODO: get the actual error here
				else if (status < 0)
					fprintf(stderr, "error: user_read\n");
			}
			if (FD_ISSET(sockfd, &rd_set)) {
				status = ircread(sockfd, ircbuf_len, ircbuf, q);
				if (status == ERR_NOSOCKET) {
					// TODO: try reconnecting
					printf("No socket, exiting.\n");
					exit(1);
				}
				else if (status < 0)
					fprintf(stderr, "unknown error: ircread\n");
			}
			if (FD_ISSET(sockfd, &wr_set)) {
				struct irc_message m;

				/* if there are messages in the queue */
				if (dequeue(q, &m)) {
					status = ircsendmessage(m);
					if (status < 0)
						fprintf(stderr, "error: ircsendmessage\n");
				}
			}
		}
		else {
			perror("select");
		}
	}
}

int user_read() {
	char buffer[MSG_MAXSIZE];
	memset(buffer, 0, sizeof(buffer));
	if (fgets(buffer, sizeof(buffer), stdin) == NULL)
		return ERR_BADGET;

	debug_print("user - %d: %s\n", sockfd, buffer);

	trim_newline(buffer);

	if (strcmp(buffer, COMMAND_EXIT) == 0)
		return CCODE_EXIT;

	return 0;
}

int ircwrite(char* s) {
	printf("SENDING THIS TO SERVER: %s\n", s);
	if (write(sockfd, s, strlen(s)) < 0) {
		return ERR_BADWRITE;
	}
	return 0;
}

int ircsendmessage(struct irc_message m) {
	char msg[MSG_MAXSIZE];
	msgtostring(msg, m);
	return ircwrite(msg);
}

int ircjoinch(const char* ch) {
	// TODO: store list of users in a data structure to allow for later querying
	return ircsendmessage(ircmessage("JOIN", 1, &ch));
}

int handlemessage(const char* message) {
	int i;
	char msg[MSG_MAXSIZE]; /* mutable copy of message */
	char* tok;
	// char prefix[CMD_MAXLEN];
	char command[CMD_MAXLEN];
	char params[MAX_PARAMS][MSG_MAXSIZE];
	char response[MSG_MAXSIZE];

	/* initialize strings */
	strncpy(msg, message, MSG_MAXSIZE-1);
	msg[MSG_MAXSIZE-1] = '\0'; // TODO: sizeof in all these vs just using each macro?
	// prefix[0] = '\0';
	command[0] = '\0';
	response[0] = '\0';
	for (i = 0; i < MAX_PARAMS; i++)
		params[i][0] = '\0';

	// TODO: should I be using strtok_r() here, or is strtok() enough?
	tok = strtok(msg, " ");
	/* if the message starts with a :, first part is the prefix */
	if (msg[0] == ':') {
		// strncpy(prefix, tok, sizeof(prefix)-1);
		tok = strtok(NULL, " ");
	}
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
			strncpy(params[i], tok+1, MSG_MAXSIZE-1);
			debug_print("params[%d]: %s\n", i, params[i]);

			/* then, if there is more, add a space and copy the rest of the msg */
			if (rest != NULL) {
				debug_print("here %d\n", i);
				params[i][toklen] = ' ';
				strncpy(params[i]+toklen+1, rest, MSG_MAXSIZE-toklen-1);
				params[i][MSG_MAXSIZE-1] = '\0';
				debug_print("here %d\n", i);
			}

			break;
		}
		debug_print("here %d\n", i);
		strncpy(params[i], tok, MSG_MAXSIZE-1);
		params[i][MSG_MAXSIZE-1] = '\0';
		i++;
	}
	
	/* command responses */
	if (strcmp(command, IRC_PING) == 0) {
		const char* param = &(params[0][0]);
		// TODO: handle errors
		struct irc_message m = ircmessage("PONG", 1, &param);
		// strcpy(m.command, "PONG");
		// strncpy(m.params[0], params[0], MSG_MAXSIZE-1);
		// m.n_params = 1;
		enqueue(q, m);
		// sprintf(response, "PONG %s\r\n", params[0]);
		// return ircwrite(response);
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
