// client.c
//
// TCP Client
//
// Connects to the TCP server and makes requests.

#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/ctcp.h"
#include "lib/error.h"

// Connect to a stream socket on the server machine
//
// Returns a socket file descriptor on success, and
// an error code on failure.
int find_server(struct addrinfo *const addropts) {
	struct addrinfo *ai;
	int status;
	for (ai = addropts; ai != NULL; ai = ai->ai_next) {
		// Create socket
		int sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock_fd == -1) {
			perror("`server_connect` socket creation failure");
			continue;
		}

		// Connect
		status = connect(sock_fd, ai->ai_addr, ai->ai_addrlen);
		if (status == -1) {
			close(sock_fd);
			perror("`server_connect` socket connection failure");
			continue;
		}

		// Found one!
		char saddr[INET6_ADDRSTRLEN];
		bzero(saddr, INET6_ADDRSTRLEN);
		inet_ntop(ai->ai_family, get_in(ai->ai_addr), saddr, ai->ai_addrlen);
		printf("Client connected to %s\n", saddr);
		return sock_fd;
	}

	// Failed to find suitable connection
	return -1;
}

// Client entry point
int main(void) {
	// Gather server network info
	struct addrinfo *ai;
	int status = get_ctcp_addrinfo(&ai);
	if (status < 0) {
		fprintf(stderr, "`get_ctcp_addrinfo` failure: %d\n", status);
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Create stream socket and connect to CTCP server
	int sock_fd = find_server(ai);
	if (sock_fd == -1) {
		fprintf(stderr, "`find_server` failure\n");
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Free extraneous network info
	freeaddrinfo(ai);

	// Get data from server
	char buf[100];
	bzero(buf, 100);
	int nbytes = recv(sock_fd, buf, 100, 0);
	if (nbytes == -1) {
		perror("`recv` failure");
		exit(CTCP_CLIENT_RECV_ERROR);
	}
	printf("Message from server: %s\n", buf);

	return CTCP_SUCCESS;
}
