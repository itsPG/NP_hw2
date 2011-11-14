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

	int shm_id;
	struct PG_extra_data
	{
		char msg[31][11][2001];
		int user_max;
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
		}
		/******************             init end           ************************/
		shmdt((void*)r);
		return r;
	}
	int create(int q){shm_id = create();}
	void link(int q)
	{
		shm_id = q;
		if ( (buf = (PG_extra_data*)shmat(q, 0, 0)) < (PG_extra_data*)0 )perror("link");
	}
	void unlink(int q)
	{
		shmdt((void*)shm_id);
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
class PG_pipe
{
public:
	struct pipe_unit
	{
		int fd[2];
		int flag;
		pipe_unit(){flag = 0;}
		
	};

	map<int, pipe_unit> p_table;
	map<int, int> relation, fd_table;

	map<int, pipe_unit>::iterator iter,iter2;
	PG_pipe()
	{
		fd_table[0] = -1;
		fd_table[1] = -1;
		fd_table[2] = -1;
	}
	void show()
	{

		map<int,int>::iterator i;
		cerr << "pid " << getpid() << endl;
		for (i = fd_table.begin(); i != fd_table.end(); ++i)
		{
			if (i->second != 0)cerr << i->first << " " << i->second << endl;
		}
	}
	void create(int q)
	{
		if (p_table.find(q) != p_table.end()) return;
		
		pipe(p_table[q].fd);
		fd_table[p_table[q].fd[0]] = p_table[q].fd[0];
		fd_table[p_table[q].fd[1]] = p_table[q].fd[1];

	}
	void close2(int q)
	{
		close(q);
		fd_table[q] = 0;
	}
	void dup22(int a,int b)
	{
		dup2(a,b);
		fd_table[b] = a;
	}
	void clean_pipe()
	{
		map<int,int>::iterator i;
		for (i = fd_table.begin(); i != fd_table.end(); ++i)
		{
			if (i->second > 2)
				close2(i->second);
		}
	}
	void fix_stdin(int q)
	{
		
		iter = p_table.find(q);
		if (iter == p_table.end()) return;

		close2(0);
		dup22(iter->second.fd[0], 0);
		close2(iter->second.fd[0]);

	}
	void fix_stdout(int q,int fix_stderr)
	{
		int aim = relation[q];
		iter = p_table.find(aim);
		if (iter == p_table.end()) return;

		close2(1);
		dup22(iter->second.fd[1], 1);
		if (fix_stderr) dup22(iter->second.fd[1], 2);
		close2(iter->second.fd[1]);
		
	}
	void fix_main(int q)
	{
		iter = p_table.find(q);
		if (iter == p_table.end()) return;

		close2(iter->second.fd[0]);
		close2(iter->second.fd[1]);
	}
	void connect(int a, int b)
	{

		relation[a] = b;
		create(b);
	}
	int chk_connect(int q){return relation[q];}
	void redirect_to_file(string fn)
	{
		int fd = open(fn.c_str(),O_WRONLY | O_CREAT | O_TRUNC,700);
		close2(1);
		dup22(fd,1);
		close2(fd);
	}
	
};
class PG_cmd
{
public:
	string cmd;
	vector<string> list;
	string PATH[101];
	vector<int> pipe_seg,pipe_seg_flag;
	map<string, string> ENV;
	int delay, delay_type;
	int seq_no, PATH_size;
	string prefix;
	string redirect_from, redirect_to; 
	int size;
	bool exit_flag, pipe_err_flag;
	
	PG_cmd()
	{
		size = 0;
		redirect_from = "";
		redirect_to = "";
		prefix = "cmd_";
		exit_flag = 0;
		pipe_err_flag = 0;
		
		PATH[0] = "bin";
		PATH[1] = "bin"; 
		PATH_size = 1;
		ENV["PATH"] = "bin";
	}
	void read()
	{
		getline(cin, cmd);
	}
	void parse()
	{
		string tmp = "";
		redirect_to = "";
		delay = 0;
		list.clear();
		
		/***********************************************************************************************/
		for (int i = 0; i < cmd.size(); i++)
		{
			if (cmd[i] == ' ' || cmd[i] == '\t' || cmd[i] == '\n' || cmd[i] == '\r')
			{
				if (tmp != "")
				{
					list.push_back(tmp);
					tmp = "";
				}
			}
			else tmp += cmd[i];
		}
		if (tmp != "")list.push_back(tmp);
		/***********************************************************************************************/
		
		for (int i = 0; i < list.size(); i++)
		{
			if(list[i] == ">")
			{
				redirect_to = list[i+1];
				list.erase(list.begin() + i); list.erase(list.begin() + i);
			}
		} 
		int end = list.size() - 1;
		if(list[end][0] =='|' || list[end][0] == '!')
		{
			if (list[end][0] == '!') pipe_err_flag = 1;
			list[end][0]=' ';
			delay = s2i(list[end]);
			list.erase(list.begin() + end);
		}
		pipe_seg.clear();
		pipe_seg_flag.clear();
		pipe_seg.push_back(-1);
		for (int i = 1; i < list.size(); i++)
		{	
			if (list[i] == "|" || list[i] == "!")
			{
				pipe_seg.push_back(i);
				pipe_seg_flag.push_back(list[i] == "!" ? 1:0);
			}			
		}
		pipe_seg.push_back(list.size());
		
		/***********************************************************************************************/
		if (list[0] == "exit")
		{
			exit_flag = 1;
		}
	}
	void show()
	{
		cerr << "size " << list.size() << endl;
		for (int i = 0; i < list.size(); i++)
		cout << list[i] << endl;
	}
	void exec(int from, int to)
	{
		if (list[from] == "setenv")
		{
			ENV[list[from+1]] = list[from+2];
			if (list[from+1] != "PATH")return;
			PATH[0] = list[from+2];
			PATH_size = 0;
			for (int i = 0; i < PATH[0].size(); i++)
			{
				PATH[++PATH_size] = "";
				while (i < PATH[0].size() && PATH[0][i] !=':')
				{
					PATH[PATH_size] += PATH[0][i];
					i++;
				} 
			}
			exit(0);
		}
		if (list[from] == "printenv")
		{
			cout << ENV[list[from+1]] << endl;
			exit(0);
		}
		struct stat statbuf;
		bool success_flag = 0;
		string aim;
		for (int i=1; i<=PATH_size; i++)
		{
			
			aim =ROOT_DIC + PATH[i] + "/" + list[from];

			if (stat(aim.c_str(), &statbuf) == -1) continue;
			else
			{
				success_flag = 1;
				break;
			}
		}
		if (!success_flag)
		{
			cerr << "Unknown command: [" << list[from] << "]." << endl;
			exit(0);
		}
		
		/***********************************************************************************************/
		
		char **tmp = new char*[1001];
		int i, cnt;
		if(from > to)return;

		for (i = from,cnt = 0; i <= to; i++,cnt++)
		{

			tmp[cnt] = new char[1001];
			strcpy(tmp[cnt], list[i].c_str());
		}
		tmp[cnt] = NULL;

		if (execv(aim.c_str(), tmp))
		{
			perror("excvp fail");
			exit(0);
		}
	}
	void exec() {exec(0,list.size()-1);}
	void exec_seg(int q){exec(pipe_seg[q]+1, pipe_seg[q+1]-1);}

};
class PG_process
{
public:
	int pid;
	int harmonics()
	{
		if (pid = fork())
		{
			return pid;
		}
		else
		{
			return 0;
		}
	}
	int Wait()
	{
		int stat;
		wait(&stat);
	}
};
class PG_TCP
{
public:
	int c_fd, l_fd, pid;
	int harmonics()
	{
		int pid, stat;
		if (pid = fork())
		{
			waitpid(pid, &stat, 0);
			return pid;
		}
		else
		{
			if (fork())
			{
				exit(0);
			}
			else return 0;
		}
	}
	void go()
	{
		struct sockaddr_in sin;
		struct sockaddr_in cin;
		
		socklen_t len;
		char buf[10000]; 
		char addr_p[INET_ADDRSTRLEN]; 
		int port = 7000;
		int n,r; 
       
		bzero(&sin, sizeof(sin)); 
		sin.sin_family = AF_INET; 
		sin.sin_addr.s_addr = INADDR_ANY; 
		sin.sin_port = htons(port);
    
		l_fd = socket(AF_INET, SOCK_STREAM, 0); 
		while(1)
		{
			r = bind(l_fd, (struct sockaddr *)&sin, sizeof(sin));
			cout << "bind: " << r << endl;
			if(r == 0)break;
			usleep(500000);
		}
		r = listen(l_fd, 10); 
		cout << "listen: " << r << endl;
		printf("waiting ...\n");
		while(1)
		{
			usleep(500000);
			c_fd = accept(l_fd, (struct sockaddr *) &cin, &len); 
			cout << "accept: " << c_fd << endl;
			if (pid = harmonics())
			{
				cout << "parent" << endl;
				close(c_fd);
			}
			else
			{
				close(l_fd);
				dup2(c_fd, 0);
				dup2(c_fd, 1);
				dup2(c_fd, 2);
				close(c_fd);
				return ;
			}
		}
	}

};
class PG_ChatRoom
{
public:
	PG_share_memory share_memory;
	PG_FIFO FIFO;
	void init()
	{
		cout << "id is " << share_memory.create(0) << endl;
	}
	void init(int q)
	{
		share_memory.link(q);
	}
	void test()
	{
		int id = ++share_memory.buf->user_max;
		cout << "this is client id " << share_memory.buf->user_max << endl;
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
void pipe_exec(PG_pipe &Elie, PG_cmd &Tio, int from, int to)
{
	PG_process Rixia;
	int pid;
	if (from == to)
	{
		Tio.exec_seg(from);
		cerr << "error!" << endl;
		return;
	}
	int fd[2];
	pipe(fd);
	if(pid = Rixia.harmonics())
	{
		close(fd[1]);
		close(0);
		dup2(fd[0],0);
		close(fd[0]);
		Rixia.Wait();
		pipe_exec(Elie,Tio,from+1,to);
		return ;
	}
	else
	{
		close(fd[0]);
		close(1);
		dup2(fd[1],1);
		if (Tio.pipe_seg_flag[from])
		{
			close(2);
			dup2(fd[1],2);
		}
		close(fd[1]);
		Tio.exec_seg(from);
		exit(0);
	}
}
int main(int argc, char* argv[])
{
	PG_ChatRoom a;
	if (argc == 2 && strcmp(argv[1],"init") == 0)
	{
		a.init();
	}
	else
	{
		int id;
		cin >> id;
		a.init(id);
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

	PG_pipe Elie;
	PG_cmd Tio;
	PG_process Rixia;
	int seq_no = 0,pid;
	PG_TCP Noel;
	chdir(ROOT_DIC);
	Noel.go();
	welcome_msg();
	
	
	while (1)
	{
		cout << "% ";
		Tio.seq_no = ++seq_no;
		Tio.read();
		Tio.parse();
		//Tio.show();	
		if (Tio.exit_flag) exit(0);
		
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
