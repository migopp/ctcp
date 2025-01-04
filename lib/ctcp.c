#include "ctcp.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

int get_ctcp_addrinfo(struct addrinfo **ai) {
	struct addrinfo hints;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;  // TCP
	hints.ai_protocol = AF_UNSPEC;	  // IPv4 or IPv6
	hints.ai_flags = AI_PASSIVE;	  // localhost

	int status = getaddrinfo(NULL, CTCP_PORT, &hints, ai);
	if (status < 0) {
		fprintf(stderr, "`get_ctcp_addrinfo` failure: %s\n",
				gai_strerror(status));
		return CTCP_CHECK_ERRNO;
	}

	return CTCP_SUCCESS;
}

void *get_in(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	} else {
		return &(((struct sockaddr_in6 *)sa)->sin6_addr);
	}
}

int hello_from(int fd, char *place) {
	if (strlen(place) >= 50) {
		return CTCP_INTERNAL_ERROR;
	}
	char buf[50];
	bzero(buf, 50);
	snprintf(buf, 50, "Hello from %s!", place);
	ssize_t nbytes = send(fd, buf, strlen(buf), 0);
	if (nbytes == -1) {
		perror("`hello_from` send failure");
		return CTCP_CHECK_ERRNO;
	}
	return CTCP_SUCCESS;
}
