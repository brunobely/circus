#ifndef HEADER_ERROR
#define HEADER_ERROR

// TODO: either write a function to get error string from error code
// TODO: or make this an enum of int and string
/***** error codes *****/
/* bad socket (e.g. not connected, not valid socket FD) */
extern const int ERR_NOSOCKET;
extern const int ERR_SOCKETWRITE;
extern const int ERR_BADREAD; /* read() */
extern const int ERR_BADWRITE; /* write() */
extern const int ERR_BADGET; /* fgets() */
extern const int ERR_BADPUT; /* fputs() */
extern const int ERR_BADMESSAGE; /* error parsing an IRC message */

#endif
