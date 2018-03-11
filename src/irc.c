// TODO: remove if unneeded
#include <stdio.h>

#include <string.h>
#include <unistd.h>

#include "ircdef.h"
#include "error.h"

int ircconnect() {
	// char buffer[MSG_MAXSIZE];
	// sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	// if (write(sockfd, buffer, strlen(buffer)) < 0)
	// 	return ERR_SOCKETWRITE;

	return 0;
}

int ircregister(int sockfd) {
	char buffer[MSG_MAXSIZE];
	sprintf(buffer, "NICK %s\r\nUSER %s 0 * :%s\r\n", NICKNAME, USERNAME, REALNAME);
	if (write(sockfd, buffer, strlen(buffer)) < 0)
		return ERR_SOCKETWRITE;

	return 0;
}

struct irc_message ircmessage(const char* command, int n_params, const char params[n_params][MSG_MAXSIZE]) {
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
