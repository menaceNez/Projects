#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int udpNewline(char *buf);

int main(int argc, char **argv)
{
	int sockfd = -1;
	int status;
	int i;
	int exitcode = EXIT_FAILURE;
	struct addrinfo hints = {.ai_socktype = SOCK_DGRAM};
	struct addrinfo *servinfo = NULL, *p;
	char buf[BUFSIZ];
	char *startPort, *endPort, *ipAddr;

	if (argc == 4)
	{
		startPort = argv[argc - 2];
		endPort = argv[argc - 1];
		ipAddr = argv[argc - 3];
	}
	else
	{
		status = -1;
		perror("Not enough input provided\n");
		exit(1);
	}
	int iterations = atoi(startPort), maxIterations = atoi(endPort) - atoi(startPort);
	// printf("%s %s %s %d %d\n", startPort, endPort, ipAddr, iterations, maxIterations);
	for (i = 0; i <= maxIterations; i++)
	{
		// reset sockfd
		sockfd = -1;
		sprintf(startPort, "%d", iterations++);
		// printf("[Start]: %s, %d %d\n", startPort, i, maxIterations);
		status = getaddrinfo(ipAddr, startPort, &hints, &servinfo);
		if (0 != status)
		{
			fprintf(stderr, "Error opening connection: %s\n", gai_strerror(status));
		}

		/* Connect to host; first option that works wins. */
		for (p = servinfo; p != NULL; p = p->ai_next)
		{
			printf("[TYPE]: %d\n", p->ai_family);
			sockfd = socket(p->ai_family, p->ai_socktype, 0);
			if (-1 == sockfd)
			{
				continue;
			}

			if (-1 == connect(sockfd, p->ai_addr, p->ai_addrlen))
			{
				close(sockfd);
				sockfd = -1;
				continue;
			}

			break;
		}
		if (-1 == sockfd)
		{
			// do nothing
		}
		else
		{
			send(sockfd, "Data push!\n", 11, 0);

			/* Read all bytes from host, and write them to stdout. */
			for (;;)
			{
				recv(sockfd, buf, BUFSIZ - 1, 0);
				// printf("Error: %d %s\n", errno, strerror(errno));
				if (udpNewline(buf) == 1)
				{ // change to a new line occurance
					printf("%s", buf);
					memset(buf,0,sizeof(buf));
					break;
				}
				else {
					break;
				}
			}
		}

	}
	exitcode = EXIT_SUCCESS;
	if (-1 != sockfd)
	{
		close(sockfd);
	}

	if (NULL != servinfo)
	{
		freeaddrinfo(servinfo);
	}

	exit(exitcode);
}
/**
 * returns 0 if no newline 1 if new line occurance
 */
int udpNewline(char *buf)
{
	size_t i;
	if (strlen(buf) > 0)
	{
		for (i = 0; i < strlen(buf); i++)
		{
			if (buf[i] == '\n')
			{
				return 1;
			}
		}
	}
	return 0;
}
