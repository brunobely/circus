#ifndef HEADER_MESSAGE_Q
#define HEADER_MESSAGE_Q

#include <stdbool.h>
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

struct qnode {
	struct irc_message message;
	struct qnode* next;
};

struct message_q {
	struct qnode* first;
	struct qnode* last;
};

/* allocates memory for a message_q, then initializes and returns it */
struct message_q* message_q();

void enqueue(struct message_q* q, struct irc_message msg);

/* returns true if successful (i.e. queue not empty), false otherwise */
bool dequeue(struct message_q* q, struct irc_message* out);

bool isempty(struct message_q* q);

/* for debugging purposes */
void printq(struct message_q* q);

void msgtostring(char* dest, struct irc_message m);

#endif
