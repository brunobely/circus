#ifndef HEADER_IRCDEF
#define HEADER_IRCDEF

#include "constants.h"

// TODO: this is very space inefficient. maybe use one MSG_MAXSIZE string
//       and indices to when each segment begins and ends, and implement
//       a parsemessage function that creates the struct, along with functions
//       to get each segment
struct irc_message {
	char prefix[MSG_MAXSIZE];
	char command[CMD_MAXLEN];
	char params[MAX_PARAMS][MSG_MAXSIZE];
	int n_params;
};

#endif
