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
	void init()
	{
		he = gethostbyname("127.0.0.1");
		int SERVER_PORT = 7000;
		for (int i = 0; i < 30; i++)
		{
			cout << i << endl;
			client_fd[i] = socket(AF_INET,SOCK_STREAM,0);
			bzero(&client_sin[i],sizeof(client_sin[i]));
			client_sin[i].sin_family = AF_INET;
			client_sin[i].sin_addr = *((struct in_addr *)he->h_addr); 
			client_sin[i].sin_port = htons(SERVER_PORT);
		}
		FD_ZERO(&r_fds);
		FD_ZERO(&a_fds);
		//FD_SET(fileno(fd), &a_fds);
		FD_SET(0, &a_fds);
	}
	void go()
	{
		cout << "go" << endl;
		memcpy(&r_fds,&a_fds,sizeof(fd_set));
		if (select(30, &r_fds, NULL, NULL, NULL) < 0)
		{
			//return ;
			cout << "error" << endl;
		}
		for (int i = 0; i < 30; i++)
		{  
			if (FD_ISSET(client_fd[i], &r_fds))
			{
				cout << "ready !! " << endl;
				//dup2(0,3510);
				//dup2(1,3511);
				//dup2(client_fd[i], 1);
				//string q;
				//while (getline(cin, q))cout << q << endl;
            }
        }
	}
};
int main()
{
	PG_FD_set a;
	a.init();
	a.go();
}
