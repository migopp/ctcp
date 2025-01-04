#ifndef CTCP

#include <netdb.h>

#define CTCP_PORT "8080"

// Fetches info for `:CTCP_PORT`
//
// If successful, the `struct addrinfo *` passed into
// `get_ctcp_addrinfo` is populated with a list of server
// addresses and `CTCP_SUCCESS` is returned. Otherwise,
// the error code is returned.
int get_ctcp_addrinfo(struct addrinfo **);

// Converts a generic `sockaddr` to:
// 		IPv4 -> `sockaddr_in`
// 		IPv6 -> `sockaddr_in6`
void *get_in(struct sockaddr *);

// Sends "Hello from <TEXT>!" to a given file descriptor
//
// If successful, `CTCP_SUCCESS` is returned. Otherwise,
// the error code is returned.
int hello_from(int, char *);

#endif	// !CTCP
