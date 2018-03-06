#ifndef HEADER_CONSTANTS
#define HEADER_CONSTANTS

/* macro so static array sizes can be set */
#define BUF_SIZE 513
#define CMD_MAXLEN 16
#define MAX_PARAMS 15

const char* SERVER = "irc.freenode.net";
const int PORT = 6667;
// const char* CHANNEL = "##bottesting19283746";
const char* NICKNAME = "Gertrudes";
const char* USERNAME = "bottybot";
// const char* HOSTNAME = "host?";
// const char* SERVERNAME = "server?";
const char* REALNAME = "Not a robot";

int sockfd;
// TODO: maybe put these inside ircread as static variables?
char ircbuf[BUF_SIZE*2];
int ircbuf_len;

/***** user commands and their command codes *****/
/* exits the program */
const char* COMMAND_EXIT = "!exit";
const int CCODE_EXIT = 1;

/***** irc commands *****/
const char* IRC_PING = "PING";

#endif
