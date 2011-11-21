#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <map>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#define ROOT_DIC "/home/PG/PG/"
#define DEBUG 0
#define PIPEMAX 100000
#define PIPEADD 10000000
#define PROG_TYPE 2
using namespace std;
void welcome_msg()
{
	cout << "****************************************" << endl;
	cout << "** Welcome to the information server. **" << endl;
	cout << "****************************************" << endl;
}
string i2s(int q)
{
	ostringstream sout;
	sout << q;
	return sout.str();
}
int s2i(string q)
{
	int r;
	istringstream ssin(q);
	ssin >> r;
	return r;
}
#include "np_hw2_shell.cpp"

struct PG_extra_data
{
	char msg[31][11][2001];
	char name[31][31];
	int user_max;
	int user_flag[31], pipe_used_flag[31];
	int m[31];
	int port[31];
	char ip[31][20];
	int fd[31];
};
PG_extra_data *ex_data;


class PG_FIFO
{
public:
	int BUFMAX;
	int fd;
	char *buf;
	string str;
	PG_FIFO()
	{
		BUFMAX = 100000;
		buf = new char[BUFMAX+1];
	}
	void fifo_open(string q)
	{
		q = "/tmp/" + q;
		mkfifo(q.c_str(),0666);
		fd = open(q.data(),O_RDWR|O_NONBLOCK);
		if(fd<0)perror("open");
		//else cout << "created " << q << " fd is " << fd << endl;
	}
	string get()
	{
		string r = "";
		int t;
		t = read(fd,buf,BUFMAX);
		buf[t] = '\0';
		r = buf;
		return r;
	}
	void put(string q)
	{
		if(write(fd,q.c_str(),q.size())<0)perror("write");
	}
	~PG_FIFO()
	{
		delete buf;
	}
};
class PG_FD_set
{
public:
	int client_fd[31];
	fd_set rfds, afds; // read / active file descriptor
	struct sockaddr_in fsin; // the from address of a client
	int alen; // from-address length
	char *buf;
	string cmd;
	int msock;
	PG_FD_set()
	{
		buf = new char[100000];
		for (int i = 1; i <= 30; i++)
		{
			client_fd[i] = 0;
		}
	}
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
	void init()
	{
		int msock = listen_port(7000);
		FD_ZERO(&afds);
		FD_SET(msock, &afds);
	}
	int login(int fd)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (client_fd[i] == 0)
			{
				client_fd[i] = fd;
				return i;
			}
		}
	}
	int fd_to_uid(int fd)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (client_fd[i] == fd)
				return i;
		}
	}
	int logout(int fd)
	{
		int uid = fd_to_uid(fd);
		close(client_fd[uid]);
		FD_CLR(client_fd[uid], &afds);
		client_fd[uid] = 0;
	}
	int go()
	{
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(1024, &rfds, NULL, NULL, NULL)<0){perror("select");}
		if (FD_ISSET(msock, &rfds))
		{
			int ssock;
			alen = sizeof(fsin);
			ssock = accept(msock, (sockaddr*)&fsin, (socklen_t*)&alen);
			if (ssock<0)perror("accept ssock");
			FD_SET(ssock, &afds);
			login(ssock);
			return 0;
		}
		for (int fd = 0; fd < 1024; ++fd)
		{
			if (fd != msock && FD_ISSET(fd, &rfds))
			{
				int t = read(fd, buf, 100000);
				//cout << "read from " << fd << endl;
					
				if (t == 0)
				{
					
					close(fd);
					FD_CLR(fd, &afds);
					perror("read fail");
					//exit(1);
				}
				else
				{
					buf[t-1] = '\0';
					cmd = buf;
					return fd_to_uid(fd);
				}
			}
		}
	}
};
class PG_global_pipe
{
public:

	PG_FIFO FIFO[31];
	void init()
	{
		string fn;
		for (int i = 1; i <= 30; i++)
		{
			fn = "test_"+i2s(i);
			FIFO[i].fifo_open(fn);
		}
	}
};
class PG_ChatRoom
{
public:
	PG_global_pipe global_pipe;

	int uid;
	int *client_fd;
	string ID[31];
	PG_ChatRoom()
	{

		global_pipe.init();
	}
	void login(string ip, int port, int q)
	{
		uid = q;
		ostringstream sout;
		sout << ip << "/" << port;
		ID[uid] = sout.str();
		broadcast("*** User \'(no name)\' entered from " + ID[uid] + ". ***", uid);
	}
	void send_msg(int q, string msg)
	{
		write(client_fd[q], msg.c_str(), 100000);
	}
	void broadcast(string q)
	{
		
		for (int i = 1; i <= 30; i++)
		{
			send_msg(i, q);
		}
	}
	void broadcast(string q, int f)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (i!=f)
				send_msg(i, q);
		}
	}
	void cmd_who()
	{
		ostringstream cout;
		for (int i = 1; i <= 30; i++)
		{
			if (ex_data->user_flag[i])
			{
				cout << i << "\t" << ex_data->name[i] << "\t" ;
				cout << ex_data->ip[i] << "/" << ex_data->port[i] << "\t";
				if (i == uid) cout << "<- me";
				cout << endl;
			}
		}
		send_msg(uid, cout.str());
	}
	void cmd_tell(int to, string &msg)
	{
		ostringstream sout;
		sout << "*** " << ex_data->name[uid] << " told you ***:" << msg;
		send_msg(to, sout.str());
		
	}
	void cmd_yell(string &msg)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (i == uid)continue;
			ostringstream sout;
			sout << "*** " << ex_data->name[uid] << " yelled " << msg;
			send_msg(i, sout.str());
			
		}
	}
	void cmd_name(string q)
	{

		strcpy(ex_data->name[uid], q.c_str());
		//)
		ostringstream sout;
		//<< 
		//cout << q.size() << endl;
		sout << "*** User from " << ID[uid] << " is named \'" << q << "\'. ***";
		broadcast(sout.str());
	}

	void logout()
	{
		ostringstream sout; 
		sout << "*** User \'" << ex_data->name[uid] << "\' left. ***";
		broadcast(sout.str(), uid);
	}
	
};
class PG_User
{
public:
	PG_pipe Elie;
	PG_cmd Tio;
	PG_process Rixia;
	int seq_no, pid;
	PG_User()
	{
		seq_no = 0;
		
	}
	void shell_main(PG_ChatRoom &ChatRoom)
	{
		chdir(ROOT_DIC);

		welcome_msg();
		//ChatRoom.login(Noel.my_ip, Noel.my_port);
		
		while (1)
		{
			cout << "% ";
			Tio.seq_no = ++seq_no;
			Tio.read();
			Tio.parse();

			if (Tio.exit_flag) 
			{
				ChatRoom.logout();
				return;
			}
		
			int pipe_to = 0;
			if(Tio.delay) 
			{
				pipe_to = seq_no + Tio.delay;
				Elie.connect(seq_no, pipe_to);
			}

			if (pid = Rixia.harmonics())
			{
				Elie.fix_main(seq_no);
				Rixia.Wait();
				if (Tio.send_to_user_flag)
				{
					int uid = ChatRoom.uid;
					string name = ex_data->name[uid], Tio_cmd = Tio.cmd;
					Tio_cmd.erase(Tio_cmd.size()-1,1);
					if	(ex_data->pipe_used_flag[uid])
					{
					}
					else
					{
						ostringstream sout;
						sout << "*** " << name << " (#" << i2s(uid) << ") just piped \'" << Tio_cmd;
						sout << "\' into his/her pipe. ***";
						ChatRoom.broadcast(sout.str());
					}
				}
			}
			else
			{
				Elie.fix_stdin(seq_no);
			
				if (Tio.pipe_err_flag)
					Elie.fix_stdout(seq_no,1);
				else
					Elie.fix_stdout(seq_no,0);

				Elie.clean_pipe();
			
				if (Tio.redirect_to != "")
					Elie.redirect_to_file(Tio.redirect_to);
			
				int uid = ChatRoom.uid;
				string name = ex_data->name[uid], Tio_cmd = Tio.cmd;
				Tio_cmd.erase(Tio_cmd.size()-1,1);

				if (Tio.recv_from_user)
				{
					//cout << "recv from user" << endl;
					if (ex_data->pipe_used_flag[Tio.recv_from_user] == 0)
					{
						cout << "*** Error: the pipe from #" << Tio.recv_from_user << " does not exist yet. ***" << endl;
						exit(0);
					}
					else 
					{
						ostringstream sout;
						sout << "*** " << name << " (#" << i2s(uid) << ") just received from the pipe #" << Tio.recv_from_user;
						sout << " by \'" << Tio_cmd << "\' ***";
						ChatRoom.broadcast(sout.str());
						ex_data->pipe_used_flag[Tio.recv_from_user] = 0;
						Elie.recv_from_user(ChatRoom.global_pipe.FIFO[Tio.recv_from_user].fd);
					}
				}
				if (Tio.send_to_user_flag)
				{

					if	(ex_data->pipe_used_flag[uid])
					{
						cout << "*** Error: your pipe already exists. ***" << endl;
						exit(0);
					}
					else
					{
						//cout << "name ~" << name << "~" << endl;
						ostringstream sout;
						sout << "*** " << name << " (#" << i2s(uid) << ") just piped \'" << Tio_cmd;
						sout << "\' into his/her pipe. ***";
						//ChatRoom.broadcast(sout.str());
						ex_data->pipe_used_flag[uid] = 1;
						Elie.send_to_user(ChatRoom.global_pipe.FIFO[ChatRoom.uid].fd, 1);
					}
				}
			
				
				if (Tio.ext_cmd != "")
				{
				
					if (Tio.ext_cmd == "who")
					{
						ChatRoom.cmd_who();
						exit(0);
					}
					if (Tio.ext_cmd == "tell")
					{
						ChatRoom.cmd_tell(Tio.ext_cmd_clientID, Tio.chat_msg);
						exit(0);
					}
					if (Tio.ext_cmd == "yell")
					{
						ChatRoom.cmd_yell(Tio.chat_msg);
						exit(0);
					}
					if (Tio.ext_cmd == "name")
					{
						ChatRoom.cmd_name(Tio.chat_msg);
						exit(0);
					}
				}
				if(Tio.pipe_seg.size() > 2)
				{
					pipe_exec(Elie, Tio, 0, Tio.pipe_seg.size()-2);
					exit(0);
					cerr << "failed to exit" << endl;
				}
				else
				{
					Tio.exec();
				}
			}
		}
	}
};

class HyperVisor
{
	PG_User User[31];
	PG_FD_set FDS;
	PG_ChatRoom ChatRoom;
	
	HyperVisor()
	{
		ex_data = new PG_extra_data;
		ChatRoom.client_fd = FDS.client_fd;
	}
	void go()
	{
		FDS.listen_port(7000);
		while (1)
		{
			int r = FDS.go();
			if (r)
			{
				User[r].shell_main(ChatRoom);
			}
		}
	}
	
};
int main(int argc, char* argv[])
{

	
}
