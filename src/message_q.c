#include <stdlib.h> /* NULL, malloc */
#include <stdio.h>
#include <string.h>

#include "message_q.h"

struct message_q* message_q() {
    struct message_q* q = malloc(sizeof(struct message_q));
    q->first = NULL;
    q->last = NULL;
    return q;
}

void enqueue(struct message_q* q, struct irc_message msg) {
    struct qnode* n = malloc(sizeof(struct qnode));
    n->message = msg;
    n->next = NULL;

    if (isempty(q)) {
        q->first = n;
        q->last = n;
    }
    else {
        q->last->next = n;
        q->last = n;
    }
}

bool dequeue(struct message_q* q, struct irc_message* out) {
    if (!isempty(q)) {
        struct qnode* f = q->first;
        *out = f->message;

        if (f->next == NULL) {
            q->first = NULL;
            q->last = NULL;
        }
        else
            q->first = f->next;

        free(f);
        return true;
    }
    return false;
}

int isempty(struct message_q* q) {
    return q->first == NULL;
}

// TODO: add indent param (int that says how many extra \t to prepend to print statements)
void printq(struct message_q* q) {
    struct qnode* n = q->first;
    printf("Queue:\n");
    while (n != NULL) {
        char msg[BUF_SIZE];
        msgtostring(msg, n->message);
        printf("  - %s\n", msg);
        n = n->next;
    }
}

void msgtostring(char* dest, struct irc_message m) {
	char params[BUF_SIZE];
	params[0] = '\0';
	
	for (int i = 0; i < m.n_params; i++) {
		if (i > 0)
			strcat(params, " ");
		strcat(params, m.params[i]);
	}

	sprintf(dest, "%s %s %s\r\n", m.prefix, m.command, params);
}
