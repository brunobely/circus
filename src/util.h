#ifndef HEADER_UTIL
#define HEADER_UTIL

//https://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_print(fmt, ...) \
        do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

void error(char* msg);

#endif