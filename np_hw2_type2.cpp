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
class PG_MSGBOX
{
	
};
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

class PG_share_memory
{
public:

	int shm_id, user_id;
	struct PG_extra_data
	{
		char msg[31][11][2001];
		char name[31][31];
		int user_max;
		int user_flag[31], pipe_used_flag[31];
		int m[31];
		int port[31];
		char ip[31][20];
		int pid[31];
	};
	PG_extra_data *buf;
	
	PG_share_memory()
	{
		
		
	}
	void logout(int uid)
	{
		buf->m[uid] = 0;
		buf->user_flag[uid] = 0;
		buf->pipe_used_flag[uid] = 0;
		strcpy(buf->name[uid], "(no name)");
		
	}
	int create()
	{
		int r = shmget(IPC_PRIVATE, 700000, 0666);
		if (r < 0){perror("create");}
		if ( (buf = (PG_extra_data*)shmat(r, 0, 0)) < (PG_extra_data*)0 )perror("link of create");
		/******************             init start           ************************/
		buf->user_max = 0;
		for (int i = 1; i <=30; i++)
		{
			logout(i);
		}
		/******************             init end           ************************/
		shmdt((void*)r);
		return r;
	}
	void link(int q)
	{
		shm_id = q;
		if ( (buf = (PG_extra_data*)shmat(q, 0, 0)) < (PG_extra_data*)0 )perror("link");
	}
	void unlink(int q)
	{
		shmdt((void*)shm_id);
	}
	void login(string ip, int port)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (buf->user_flag[i] == 0)
			{
				user_id = i;
				buf->user_max = i;
				buf->user_flag[i] = 1;
				strcpy(buf->ip[i], ip.c_str());
				buf->port[i] = port;
				buf->pid[i] = getpid();
				if (DEBUG)cout << "ip : " << buf->ip[i] << endl;
				if (DEBUG)cout << "port : " << buf->port[i] << endl;
				return;
			}
		}
	}
	
	void send_msg(int to,string q)
	{
		if (!buf->user_flag[to])return;
		if (buf->m[to] >= 10)return;
		strcpy(buf->msg[to][ ++buf->m[to] ], q.c_str());
		for (int i = 1; i <= 30; i++)
		{
			if (buf->user_flag[i])
			{
				kill(buf->pid[i], SIGUSR1);
				//if (kill(buf->pid[i], SIGUSR1) == -1)perror("fail to send signal");
					
			}
		}
	}
	
	string recv_msg(int id)
	{
		string r = "", t;
		for (int i = 1; i <= buf->m[id]; i++)
		{
			t = buf->msg[id][i];
			r += t + "\n";
		}
		buf->m[id] = 0;
		return r;
	}
	void recv_msg()
	{
		cout << recv_msg(user_id);
	}
	void test()
	{
		int t; cin >> t;
		if(t==-1)
			shm_id = create();
		else shm_id = t;
		cout << "shm_id " << shm_id << endl;
		link(shm_id);
		if (t != -1)
		{
			cout << recv_msg(7) << endl;
			return;
		}
		string tmp;
		cin >> tmp;
		send_msg(7,"****************");
		send_msg(7,tmp);
		send_msg(7,"!!!!!!!!!!!!!!!!");
		//strcpy(buf->msg[1][2], tmp.c_str());
		//buf->m[13] = 3510;
		unlink(shm_id);
	}
	
};
PG_share_memory share_memory;
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
	PG_FIFO FIFO;
	int uid;
	string ID;
	void init_firsttime()
	{
		int t = share_memory.create();
		if (DEBUG)cout << "id is " << t << endl;
		ofstream fout("/tmp/PG_autoid");
		fout << t << endl;
		fout.close();		
	}
	//void broadcast(string q, int f);
	//void broadcast(string q);
	void init(string ip, int port)
	{
		int t;
		ifstream fin("/tmp/PG_autoid");
		fin >> t;
		fin.close();
		if (DEBUG)cout << "shmid is" << t << endl;
		if (DEBUG)cout << "ChatRoom will init with " << ip << " / " << port << endl;
		ostringstream sout;
		sout << ip << "/" << port;
		ID = sout.str();
		share_memory.link(t);
		share_memory.login(ip,port);
		uid = share_memory.user_id;
		global_pipe.init();
		//cout << "the ID of this user is " << ID << endl;
		broadcast("*** User \'(no name)\' entered from " + ID + ". ***", uid);
	}
	string recv_msg()
	{
		return share_memory.recv_msg(uid);
	}
	void broadcast(string q)
	{
		cout.flush();
		//cout << "broadcast ==== " << q << endl;
		for (int i = 1; i <= 30; i++)
		{
			share_memory.send_msg(i, q);
		}
	}
	void broadcast(string q, int f)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (i!=f)
				share_memory.send_msg(i, q);
		}
	}
	void cmd_who()
	{
		for (int i = 1; i <= 30; i++)
		{
			if (share_memory.buf->user_flag[i])
			{
				cout << i << "\t" << share_memory.buf->name[i] << "\t" ;
				cout << share_memory.buf->ip[i] << "/" << share_memory.buf->port[i] << "\t";
				if (i == uid) cout << "<- me";
				cout << endl;
			}
		}
	}
	void cmd_tell(int to, string &msg)
	{
		ostringstream sout;
		sout << "*** " << share_memory.buf->name[uid] << " told you ***:" << msg;
		share_memory.send_msg(to, sout.str());
		
	}
	void cmd_yell(string &msg)
	{
		for (int i = 1; i <= 30; i++)
		{
			if (i == uid)continue;
			ostringstream sout;
			sout << "*** " << share_memory.buf->name[uid] << " yelled " << msg;
			share_memory.send_msg(i, sout.str());
			
		}
	}
	void cmd_name(string q)
	{
		strcpy(share_memory.buf->name[uid], q.c_str());
		//)
		ostringstream sout;
		//<< 
		//cout << q.size() << endl;
		sout << "*** User from " << ID << " is named \'" << q << "\'. ***";
		broadcast(sout.str());
	}
	void test()
	{
		int id = share_memory.user_id;
		uid = id;
		cout << "this is client id " << share_memory.user_id << endl;
		while (1)
		{
			string str;
			str = share_memory.recv_msg(id);
			cout << "-------------msg list --------" << endl;
			cout << str;
			cout << "------------   end    --------" << endl;
			cout << "select id to talk ";
			int t;
			cin >> t;
			cout << "type some words : ";
			cin.ignore();
			getline(cin, str);
			share_memory.send_msg(t, str);
		}
	}
	void logout()
	{
		ostringstream sout; 
		sout << "*** User \'" << share_memory.buf->name[uid] << "\' left. ***";
		broadcast(sout.str(), uid);
		share_memory.logout(uid);
		
	}
	~PG_ChatRoom()
	{
		share_memory.buf->user_max--;
	}
};
void handler(int signo)
{
	//cout << "handler start" << endl;
	share_memory.recv_msg();
	//cout << "handler end" << endl;
}
void shell_main(PG_ChatRoom &ChatRoom)
{
	PG_pipe Elie;
	PG_cmd Tio;
	PG_process Rixia;
	int seq_no = 0,pid;
	PG_TCP Noel;
	chdir(ROOT_DIC);
	Noel.go();
	welcome_msg();
	
	if (PROG_TYPE == 1)
		ChatRoom.init(Noel.my_ip, Noel.my_port);
	else
		ChatRoom.init(Noel.my_ip, getpid());
	
	if (signal(SIGUSR1,handler) == SIG_ERR)
	{
		perror("cant regist SIGUSR1");
	}
	while (1)
	{
		//cout << "-----msg_start-----" << endl;
		//////cout << ChatRoom.recv_msg();
		//cout << "-----msg_end-----" << endl;
		//cout << " / pid : " << getpid();
		cout << "% ";
		Tio.seq_no = ++seq_no;
		Tio.read();
		//////cout << ChatRoom.recv_msg();
		Tio.parse();
		//Tio.show();	
		if (Tio.exit_flag) 
		{
			ChatRoom.logout();
			exit(0);
		}
		
		int pipe_to = 0;
		if(Tio.delay) 
		{
			pipe_to = seq_no + Tio.delay;
			Elie.connect(seq_no, pipe_to);
		}
		//cout << "before fork" << endl;
		if (pid = Rixia.harmonics())
		{
			Elie.fix_main(seq_no);
			Rixia.Wait();
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
			string name = share_memory.buf->name[uid], Tio_cmd = Tio.cmd;
			Tio_cmd.erase(Tio_cmd.size()-1,1);

			if (Tio.recv_from_user)
			{
				//cout << "recv from user" << endl;
				if (share_memory.buf->pipe_used_flag[Tio.recv_from_user] == 0)
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
					share_memory.buf->pipe_used_flag[Tio.recv_from_user] = 0;
					Elie.recv_from_user(ChatRoom.global_pipe.FIFO[Tio.recv_from_user].fd);
				}
			}
			if (Tio.send_to_user_flag)
			{

				if	(share_memory.buf->pipe_used_flag[uid])
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
					ChatRoom.broadcast(sout.str());
					share_memory.buf->pipe_used_flag[uid] = 1;
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
int main(int argc, char* argv[])
{

	PG_ChatRoom ChatRoom;
	if (argc == 2 && strcmp(argv[1],"init") == 0)
	{
		ChatRoom.init_firsttime();
	}
	shell_main(ChatRoom);
	
	
	
	if (argc == 2 && strcmp(argv[1],"init") == 0)
	{
		ChatRoom.init_firsttime();
	}
	else
	{
		//ChatRoom.init();
		cout << "$$$$$$" << endl;
		ChatRoom.test();
	}
	return 0;
	
	/*
	PG_share_memory PGB;
	PGB.test();
	return 0;
	PG_global_pipe PG;
	PG.test();
	return 0;
	*/
	shell_main(ChatRoom);
}
