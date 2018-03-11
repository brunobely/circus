# CIRCUS
## TODO list and known bugs

- [ ] Gracefully exit (close sockets, FDs, etc.)
  * see man page for `atexit(3)`

- [ ] On socket close (specifically noticed after a PING without response), select will keep marking sockfd as ready to read, but the read results in an error, causing an infinite loop
  * try reconnecting to socket if that happens

- [ ] Call `shutdown()` on sockets before closing?

- [ ] Could a server ever send an invalid message? If so, validate messages

- [ ] Improve `ircread()` code to handle the case that no `\n`/`\r` is found within `BUF_SIZE * 2` characters
  * [https://stackoverflow.com/questions/6090594/c-recv-read-until-newline-occurs](See the accepted answer here)

- [x] Implement a "message queue" that will hold messages to be sent to the server in the order they were issued whenever the server socket is ready for writing

- [ ] Use recursive `make`
  * [See this GNU Make Reference Manual page](https://www.gnu.org/software/make/manual/html_node/Recursion.html)

- [ ] Create `irc_svconn` and `irc_chconn` that represent, respectively, a server connection and a channel connection
