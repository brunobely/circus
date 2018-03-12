#ifndef HEADER_IRC
#define HEADER_IRC

#include "constants.h"
#include "message_q.h"

/* Closes sockfd and sets its value to -1 */
void ircclosesocket(int* sockfd);
int ircconnect(const char* server, const int port);
// TODO: remove all of these parameters and take a struct sv_conn. and remove message_q include
int ircread(int sockfd, int ircbuf_len, char ircbuf[ircbuf_len], struct message_q* q);
int ircregister(int sockfd);
struct irc_message ircmessage(const char* command, int n_params, const char** params);
struct irc_message ircmessagep(const char* prefix, const char* command, int n_params, const char** params);

#endif
