#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_LINE 100
    
#include <iostream>
using namespace std;
int main(int argc, char *argv[])
{
     struct sockaddr_in sin;
     char buf[MAX_LINE];
     int sfd;
		int port = 7;
     char *str = "test sting"; 
    
     if (argc > 1)
          str = argv[1];
    
     bzero(&sin, sizeof(sin));
     sin.sin_family = AF_INET;
	
     inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr); 
     sin.sin_port = htons(port); 
    
     sfd = socket(AF_INET, SOCK_STREAM, 0); 

     connect(sfd, (sockaddr *)&sin, sizeof(sin));
     cout << sfd << endl; 
     dup2(sfd,0);
     dup2(sfd,1);
     cout << "test" << endl;
     string t;
     cin >> t;
     cerr << t << endl;
     
	//perror("connetc");
     //write(sfd, str, strlen(str) + 1); 
	
     //read(sfd, buf, MAX_LINE); 

     printf("recive from server: %s\n", buf);

     close(sfd); 
     
	return 0;
}
