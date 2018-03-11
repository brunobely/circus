#ifndef HEADER_IRC
#define HEADER_IRC

#include "constants.h"

int ircconnect(const char* server, const int port);
int ircregister(int sockfd);
struct irc_message ircmessage(const char* command, int n_params, const char* params[n_params]);
struct irc_message ircmessagep(const char* prefix, const char* command, int n_params, const char** params);

#endif
