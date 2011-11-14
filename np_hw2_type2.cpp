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
class PG_global_pipe
{
public:
	PG_global_pipe()
	{
		//mkfifo("test",0666);
	}
	PG_FIFO FIFO[51];
	void create_global_pipes()
	{
		string fn;
		for (int i = 1; i <= 30; i++)
		{
			fn = "test_"+i2s(i);
			FIFO[i].fifo_open(fn);
		}
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
class PG_share_memory
{
public:

	int shm_id, user_id;
	struct PG_extra_data
	{
		char msg[31][11][2001];
		int user_max;
		int user_flag[31];
		int m[31];
		int port[31];
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
	void login()
	{
		for (int i = 1; i <= 30; i++)
		{
			if (buf->user_flag[i] == 0)
			{
				user_id = i;
				buf->user_max = i;
				buf->user_flag[i] = 1;
				return;
			}
		}
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

class PG_ChatRoom
{
public:
	PG_share_memory share_memory;
	PG_FIFO FIFO;
	void init_firsttime()
	{
		int t = share_memory.create();
		cout << "id is " << t << endl;
		ofstream fout("/tmp/PG_autoid");
		fout << t << endl;
		fout.close();		
	}
	void init()
	{
		int t;
		ifstream fin("/tmp/PG_autoid");
		fin >> t;
		fin.close();
		//cout << "get " << t << endl;
		
		share_memory.link(t);
		share_memory.login();
		
	}
	void test()
	{
		int id = share_memory.user_id;
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
			cout << "type some words";
			cin.ignore();
			getline(cin, str);
			share_memory.send_msg(t, str);
		}
	}
	~PG_ChatRoom()
	{
		share_memory.buf->user_max--;
	}
};


int main(int argc, char* argv[])
{
	PG_ChatRoom a;
	if (argc == 2 && strcmp(argv[1],"init") == 0)
	{
		a.init_firsttime();
	}
	else
	{
		a.init();
		cout << "$$$$$$" << endl;
		a.test();
	}
	return 0;
	
	PG_share_memory PGB;
	PGB.test();
	return 0;
	PG_global_pipe PG;
	PG.test();
	return 0;

	shell_main();
}
