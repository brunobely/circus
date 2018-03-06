#ifndef HEADER_MESSAGE_Q
#define HEADER_MESSAGE_Q

#include <stdbool.h>
#include "constants.h"

// TODO: this is very space inefficient. maybe use one BUF_SIZE string
//       and indices to when each segment begins and ends, and implement
//       a parsemessage function that creates the struct, along with functions
//       to get each segment
struct irc_message {
	char prefix[BUF_SIZE];
	char command[CMD_MAXLEN];
	char params[MAX_PARAMS][BUF_SIZE];
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

int isempty(struct message_q* q);

/* for debugging purposes */
void printq(struct message_q* q);

#endif
