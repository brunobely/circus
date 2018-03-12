// TODO: remove if unneeded
#include <stdio.h>

#include <string.h>
#include <unistd.h>

#include "irc.h"
#include "ircdef.h"
#include "error.h"
#include "util.h"

void ircclosesocket(int* sockfd) {
	close(*sockfd);
	*sockfd = -1;
}

int ircconnect() {
	// char buffer[MSG_MAXSIZE];
	// sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	// if (write(sockfd, buffer, strlen(buffer)) < 0)
	// 	return ERR_SOCKETWRITE;

	return 0;
}

/*
   ircread uses the persistent buffer ircbuf to store what is read from
   the socket across calls. Any time a newline is found, the function
   extracts that message from the buffer and calls handlemessage() with
   it. It does so until there are no more new lines in the permanent
   buffer.
*/
int ircread(int sockfd, int ircbuf_len, char ircbuf[ircbuf_len], struct message_q* q) {
	int n;
	char buffer[MSG_MAXSIZE];
	char message[MSG_MAXSIZE];
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
		ircclosesocket(&sockfd);
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

int ircregister(int sockfd) {
	char buffer[MSG_MAXSIZE];
	sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	if (write(sockfd, buffer, strlen(buffer)) < 0)
		return ERR_SOCKETWRITE;

	return 0;
}

struct irc_message ircmessage(const char* command, int n_params, const char** params) {
	struct irc_message m;
	strncpy(m.command, command, CMD_MAXLEN-1);
	m.command[CMD_MAXLEN-1] = '\0';
	for (int i = 0; i < n_params; i++) {
		strncpy(m.params[i], params[i], MSG_MAXSIZE-1);
		m.params[i][MSG_MAXSIZE-1] = '\0';
	};
	m.n_params = n_params;
	return m;
}

struct irc_message ircmessagep(const char* prefix, const char* command, int n_params, const char** params) {
	struct irc_message m;
	strncpy(m.prefix, prefix, MSG_MAXSIZE-1);
	m.prefix[MSG_MAXSIZE-1] = '\0';
	strncpy(m.command, command, CMD_MAXLEN-1);
	m.command[CMD_MAXLEN-1] = '\0';
	for (int i = 0; i < n_params; i++) {
		strncpy(m.params[i], params[i], MSG_MAXSIZE-1);
		m.params[i][MSG_MAXSIZE-1] = '\0';
	};
	m.n_params = n_params;
	return m;
}
