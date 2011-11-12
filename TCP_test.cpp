/* TCPmechod.c - main, echo */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define QLEN 5 /* maximum connection queue length */
#define BUFSIZE 4096
#include "PG_include.cpp"
#include <iostream>
using namespace std;
int echo(int fd);

int  main(int argc, char *argv[])
{
	cout << "start " << argc << endl;
	char *service = "echo"; /* service name or port number */
	struct sockaddr_in fsin; /* the from address of a client*/
	int msock; /* master server socket */
	fd_set rfds; /* read file descriptor set */
	fd_set afds; /* active file descriptor set */
	int alen; /* from-address length */
	int fd, nfds;
	switch (argc)
	{
		case 1:
			break;
		case 2:
			service = argv[1];
			break;
		default:
			cout << "error" << endl;
			errexit("usage: TCPmechod [port]\n");
	}
	msock = passiveTCP(service, QLEN);
	nfds = getdtablesize();
	FD_ZERO(&afds);
	FD_SET(msock, &afds);
	while (1)
	{
		cout << "in" << endl;
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0)
			errexit("select: %s\n", strerror(errno));
		
		if (FD_ISSET(msock, &rfds))
		{
			int ssock;
			alen = sizeof(fsin);
			ssock = accept(msock, (sockaddr *)&fsin, (socklen_t *)&alen);
			if (ssock < 0)
			errexit("accept: %s\n", strerror(errno));
			FD_SET(ssock, &afds);
		}
		
		for (fd=0; fd<nfds; ++fd)
			if (fd != msock && FD_ISSET(fd, &rfds))
				if (echo(fd) == 0)
				{
					(void) close(fd);
					FD_CLR(fd, &afds);
				}
	}
}
int echo(int fd)
{	
	char buf[BUFSIZE];
	int cc;
	cc = read(fd, buf, sizeof buf);
	if (cc < 0)
	errexit("echo read: %s\n", strerror(errno));
	if (cc && write(fd, buf, cc) < 0)
	errexit("echo write: %s\n", strerror(errno));
	return cc;
}	
	
