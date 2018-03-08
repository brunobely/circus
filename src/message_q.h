#ifndef HEADER_MESSAGE_Q
#define HEADER_MESSAGE_Q

#include <stdbool.h>
#include "ircdef.h"

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
