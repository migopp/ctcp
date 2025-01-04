// server.c
//
// TCP Server
//
// Opens a stream socket, then listens on it and serves
// client requests.

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "lib/ctcp.h"
#include "lib/error.h"

#define SERVER_BACKLOG 10

struct addrinfo servinfo;

// Main server loop; accepts incoming connections, then
// forks to handle them.
void server_loop(int sock_fd) {
	// Display server details
	char saddr[INET6_ADDRSTRLEN];
	bzero(saddr, INET6_ADDRSTRLEN);
	inet_ntop(servinfo.ai_family, get_in(servinfo.ai_addr), saddr,
			  sizeof(saddr));
	printf("CTCP server running @ %s\n", saddr);

	struct sockaddr_storage caddr;
	socklen_t caddr_len = sizeof(caddr);
	for (;;) {
		// Accept a connection
		int conn_fd = accept(sock_fd, (struct sockaddr *)&caddr, &caddr_len);
		if (conn_fd == -1) {
			perror("`server_loop` acceptance failure");
			return;
		}

		// Display connection
		char caddrs[INET6_ADDRSTRLEN];
		bzero(caddrs, INET6_ADDRSTRLEN);
		inet_ntop(caddr.ss_family, get_in((struct sockaddr *)&caddr), caddrs,
				  sizeof(caddrs));
		printf("CTCP connection -> %s\n", caddrs);

		// Serve request
		if (!fork()) {
			close(sock_fd);
			int status = hello_from(conn_fd, saddr);
			close(conn_fd);
			exit(status);
		}
		close(conn_fd);
	}
}

// Bind to a stream socket on the server machine
//
// Returns the socket file descriptor on success, and
// an error code on failure.
int find_server(struct addrinfo *const addropts) {
	// Iterate over the network options, and bind to the first
	// one we are able to
	struct addrinfo *ai;
	int status;
	for (ai = addropts; ai != NULL; ai = ai->ai_next) {
		// Create socket
		int sock_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock_fd == -1) {
			perror("`server_connect` socket creation failure");
			continue;
		}

		// Enable address reuse
		int y = 1;
		status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
		if (status == -1) {
			close(sock_fd);
			perror("`server_connect` address reuse failure");
			continue;
		}

		// Bind to socket
		status = bind(sock_fd, ai->ai_addr, ai->ai_addrlen);
		if (status == -1) {
			close(sock_fd);
			perror("`server_connect` socket binding failure");
			continue;
		}

		// Found one!
		servinfo = *ai;
		return sock_fd;
	}

	// Failed to find a suitable connection
	return -1;
}

// Waits for exiting child processes (request handlers),
// so as to prevent zombie processes.
void sigchld_handler(int sig) {
	// `waitpid` can overwrite errno, so save and restore
	int old_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0);
	printf("Child process reaped: %d\n", sig);
	errno = old_errno;
}

// Server entry point
int main(void) {
	// Gather network info for server
	//
	// Run off of this machine, port `CTCP_PORT` (likely 8080)
	struct addrinfo *ai;
	int status = get_ctcp_addrinfo(&ai);
	if (status < 0) {
		fprintf(stderr, "`get_ctcp_addrinfo` failure: %d\n", status);
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Create and bind to a stream socket
	int sock_fd = find_server(ai);
	if (sock_fd == -1) {
		fprintf(stderr, "`find_server` failure\n");
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Free exraneous network info
	freeaddrinfo(ai);

	// Listen for incoming connections
	if (listen(sock_fd, SERVER_BACKLOG) == -1) {
		close(sock_fd);
		perror("`listen` failure");
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Setup SIGCHLD handler
	struct sigaction sa;
	bzero(&sa, sizeof(struct sigaction));
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		close(sock_fd);
		perror("`sigchld_handler` setup failure");
		exit(CTCP_SERVER_SETUP_ERROR);
	}

	// Run server loop
	server_loop(sock_fd);

	// Close server when done
	printf("Closing server.\n");
	return CTCP_SUCCESS;
}
