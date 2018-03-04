# CIRCUS
## TODO list and known bugs

-[ ] Gracefully exit (close sockets, FDs, etc.)
  * see man page for `atexit(3)`

-[ ] On socket close (specifically noticed after a PING without response), select will keep marking sockfd as ready to read, but the read results in an error, causing an infinite loop
  * try reconnecting to socket if that happens

-[ ] Call `shutdown()` on sockets before closing?