#ifndef HEADER_CONSTANTS
#define HEADER_CONSTANTS

/* macro so static array sizes can be set */
// TODO: maybe rename to MSG_MAXSIZE
#define BUF_SIZE 513
#define CMD_MAXLEN 16
#define MAX_PARAMS 15

// https://stackoverflow.com/a/3088268/3403247
extern const char* SERVER;
extern const int PORT;
extern const char* CHANNEL;
extern const char* NICKNAME;
extern const char* USERNAME;
extern const char* HOSTNAME;
extern const char* SERVERNAME;
extern const char* REALNAME;

/***** user commands and their command codes *****/
/* exits the program */
extern const char* COMMAND_EXIT;
extern const int CCODE_EXIT;

/***** irc commands *****/
extern const char* IRC_PING;

#endif
