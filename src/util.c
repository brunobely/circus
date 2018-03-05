#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

/*
   PARAMS:
   - s: a NUL-terminated string
*/
void trim_newline(char* s) {
	int l = strlen(s);
	if (s[l-1] == '\n')
		s[l-1] = '\0';
}

int trim_left(char* s) {
	int len;
	int i = 0;
	while ((s[0] == '\n' || s[0] == '\r') && (len = strlen(s)) > 0) {
		memmove(s, s+1, len);
		s[len]='\0';
		i++;
	}

	return i;
}
