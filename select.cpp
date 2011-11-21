#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

#include <iostream>
using namespace std;
class PG_FD_set
{
public:
	fd_set r_fds, a_fds;
	int client_fd[31];
	struct sockaddr_in client_sin[31];
	int len;
	int SERVER_PORT;
	struct hostent *he;
	FILE *fd;
	int listen_port(int port)
	{
		int l_fd;
		//int local_debug = 1;
		struct sockaddr_in sin;
		struct sockaddr_in c_in;
		
		socklen_t len;
		//char buf[10000]; 
		char addr_p[INET_ADDRSTRLEN]; 
		int n,r; 
       
		bzero(&sin, sizeof(sin)); 
		sin.sin_family = AF_INET; 
		sin.sin_addr.s_addr = INADDR_ANY; 
		sin.sin_port = htons(port);
    
		l_fd = socket(AF_INET, SOCK_STREAM, 0); 
		while(1)
		{
			r = bind(l_fd, (struct sockaddr *)&sin, sizeof(sin));
			cout << "bind: " << sin.sin_port << endl;
			if(r == 0)break;
			usleep(500000);
		}
		r = listen(l_fd, 10); 
		//cout << "listen: " << r << endl;
		//printf("waiting ...\n");
		return l_fd;
	}

	void go()
	{
		
		fd_set rfds, afds; // read / active file descriptor
		struct sockaddr_in fsin; // the from address of a client
		int alen; // from-address length
		char buf[10000];
		int msock = listen_port(7000);
		FD_ZERO(&afds);
		FD_SET(msock, &afds);
		while (1)
		{
			usleep(100000);
			memcpy(&rfds, &afds, sizeof(rfds));
			if (select(1024, &rfds, NULL, NULL, NULL)<0){perror("select");}
			if (FD_ISSET(msock, &rfds))
			{
				int ssock;
				alen = sizeof(fsin);
				ssock = accept(msock, (sockaddr*)&fsin, (socklen_t*)&alen);
				if (ssock<0)perror("accept ssock");
				FD_SET(ssock, &afds);
			}
			for (int fd = 0; fd < 1024; ++fd)
			{
				if (fd != msock && FD_ISSET(fd, &rfds))
				{
					int t = read(fd, buf,sizeof(buf));
					//cout << "read from " << fd << endl;
					if (t == 0)
					{
						close(fd);
						FD_CLR(fd, &afds);
					}
					else 
					{
						buf[t-1] = '\0';
						cout << fd << "| " << buf << endl;
					}
				}
			}
		}
		
	}
	
};
int main()
{
	PG_FD_set a;
	//a.listen_port(7000);
	a.go();
}
