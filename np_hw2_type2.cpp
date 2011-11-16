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
#define ROOT_DIC "/home/PG/PG/"
#define DEBUG 0
#define PIPEMAX 100000
#define PIPEADD 10000000
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
		int user_max;
		int user_flag[31], pipe_used_flag[31];
		int m[31];
		int port[31];
		char ip[31][20];
	};
	PG_extra_data *buf;
	
	PG_share_memory()
	{
		
		
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
			buf->m[i] = 0;
			buf->user_flag[i] = 0;
			buf->pipe_used_flag[i] = 0;
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
				cout << "login " << endl;
				user_id = i;
				buf->user_max = i;
				buf->user_flag[i] = 1;
				strcpy(buf->ip[i], ip.c_str());
				buf->port[i] = port;
				cout << "ip : " << buf->ip[i] << endl;
				cout << "port : " << buf->port[i] << endl;
				return;
			}
		}
	}
	void logout(int uid)
	{
		buf->user_flag[uid] = 0;
		buf->m[uid] = 0;
		
	}
	void send_msg(int to,string q)
	{
		if(to == 0)
		{
			for (int i = 1; i <= buf->user_max; i++)
				send_msg(i,q);
			return;
		}
		if (buf->m[to] >= 10)return;
		strcpy(buf->msg[to][ ++buf->m[to] ], q.c_str());
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
		//cout << r << endl;
		return r;
		
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

	PG_global_pipe()
	{

		//mkfifo("test",0666);
	}
	PG_FIFO FIFO[31];
	
	void create_global_pipes()
	{
		string fn;
		for (int i = 1; i <= 30; i++)
		{
			fn = "test_"+i2s(i);
			FIFO[i].fifo_open(fn);
		}
	}
	void fix_stdout(int uid)
	{
		close(1);
		dup2(FIFO[uid].fd,1);
		close(FIFO[uid].fd);
	}

	void test()
	{

		create_global_pipes();
		FIFO[3].put("21684698463608468409048\n");
		FIFO[5].put("abc");
		FIFO[3].put("111111111111111111111111111111111");
		FIFO[3].put("111111111111111111111111111111111");
		FIFO[3].put("111111111111111111111111111111111");
		FIFO[3].put("111111111111111111111111111111111");
		FIFO[3].put("111111111111111111111111111111111");
		FIFO[3].put("111111111111111111111111111111111");
		cout << FIFO[3].get() << endl;
		FIFO[3].put("efefefefef");
		cout << FIFO[3].get() << endl;
		
	}

};

class PG_ChatRoom
{
public:
	PG_global_pipe global_pipe;
	PG_FIFO FIFO;
	int uid;
	void init_firsttime()
	{
		int t = share_memory.create();
		cout << "id is " << t << endl;
		ofstream fout("/tmp/PG_autoid");
		fout << t << endl;
		fout.close();		
	}
	void init(string ip, int port)
	{
		int t;
		ifstream fin("/tmp/PG_autoid");
		fin >> t;
		fin.close();
		cout << "shmid is" << t << endl;
		cout << "ChatRoom will init with " << ip << " / " << port << endl;
		share_memory.link(t);
		share_memory.login(ip,port);
		uid = share_memory.user_id;
	}
	string recv_msg()
	{
		return share_memory.recv_msg(uid);
	}
	void cmd_who()
	{
		cout << "ip : " << share_memory.buf->ip[uid] << endl;
		cout << "port : " << share_memory.buf->port[uid] << endl;
		cout << "uid : " << uid << endl;
	}
	void cmd_tell(int to, string &msg)
	{
		share_memory.send_msg(to, msg);
	}
	void cmd_yell(int to, string &msg)
	{
		for (int i = 1; i <= share_memory.buf->user_max; i++)
		{
			if (i == uid)continue;
			cmd_tell(i,msg);
			
		}
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
		share_memory.logout(uid);
		
	}
	~PG_ChatRoom()
	{
		share_memory.buf->user_max--;
	}
};

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
	
	ChatRoom.init(Noel.my_ip, Noel.my_port);
	
	
	while (1)
	{
		cout << "-----msg_start-----" << endl;
		cout << ChatRoom.recv_msg();
		cout << "-----msg_end-----" << endl;
		cout << " / pid : " << getpid();
		cout << "% ";
		Tio.seq_no = ++seq_no;
		Tio.read();
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

			if (Tio.recv_from_user)
			{
				if (share_memory.buf->pipe_used_flag[Tio.recv_from_user] == 0)
					cout << "pipe dosent exist" << endl;
				else 
				{
					share_memory.buf->pipe_used_flag[Tio.recv_from_user] = 0;
					Elie.recv_from_user(ChatRoom.global_pipe.FIFO[Tio.recv_from_user].fd);
				}
			}
			if (Tio.send_to_user_flag)
			{

				if	(share_memory.buf->pipe_used_flag[uid])
					cout << "pipe already exist" << endl;
				else
				{
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
					ChatRoom.cmd_yell(Tio.ext_cmd_clientID, Tio.chat_msg);
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
