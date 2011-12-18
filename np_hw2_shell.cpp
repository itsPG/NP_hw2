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
	void recv_from_user(int fd)
	{
		//cout << "receiving fd: " << fd << endl;
		close2(0);
		dup22(fd,0);
	}
	void send_to_user(int fd, int type)
	{
		// q == 1 -> pipe stdout
		// q == 2 -> pipe stdout and stderr
		if (type == 1)
		{
			//cout << "duping fd : " << fd << endl;
			close2(1);
			dup22(fd,1);
			close2(fd);
		}
		if (type == 2)
		{
			close2(1); close2(2);
			dup22(fd,1); dup22(fd,2);
			close2(fd);
		}
	}
};
class PG_cmd
{
public:
	string cmd;
	vector<string> list;
	string PATH[101]; // declare where to exec command
	vector<int> pipe_seg,pipe_seg_flag;  // store parsed command, and type flag (to pipe cerr or not)
	map<string, string> ENV; // an ENV table
	int delay, delay_type; // |n , !n
	int send_to_user_flag;
	// 0:normal, 1:send stdout to user's pipe, 2:send stdout and stderr to user's pipe; 
	int recv_from_user; 
	int seq_no, PATH_size;
	int ext_cmd_clientID;
	bool success_flag;

	string redirect_from, redirect_to, ext_cmd;
	string chat_msg; 
	int size;
	bool exit_flag, pipe_err_flag;
	
	int target_no;
	PG_cmd()
	{
		size = 0;
		redirect_from = "";
		redirect_to = "";
		exit_flag = 0;
		pipe_err_flag = 0;
		send_to_user_flag = 0;
		recv_from_user = 0;
		
		PATH[0] = "bin:.";
		PATH[1] = "bin"; 
		PATH[2] = ".";
		PATH_size = 2;
		ENV["PATH"] = "bin:.";
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
		recv_from_user = 0;
		send_to_user_flag = 0;
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
			/*
				this code assume those pipe commands (i.e. > file name >| >!)
				will appear at the last list
			*/

			if (list[i] == ">")
			{
				redirect_to = list[i+1];
				list.erase(list.begin() + i); list.erase(list.begin() + i);
			}

			if (list[i] == ">|")
			{
				list.erase(list.begin() + i);
				send_to_user_flag = 1;
			}
			if (list[i] == ">!")
			{
				list.erase(list.begin() + i);
				send_to_user_flag = 2;

			}

			if (list[i][0] == '<')
			{
				//cout << "in!" << endl;
				list[i].erase(0,1);
				istringstream ssin(list[i]);
				ssin >> recv_from_user;

				list.erase(list.begin() + i);
			}
		} 
		/*VVVVVVVVVV                     processing extend command                 VVVVVVVVVV*/
		ext_cmd = "";
		
		if (list[0] == "tell")
		{
			istringstream ssin(cmd);
			ssin >> ext_cmd >> ext_cmd_clientID;
			while(ssin.peek()==' ')ssin.get();
			getline(ssin, chat_msg);
			chat_msg.erase(chat_msg.size()-1,1);
			
		}
		if (list[0] == "change")
		{
			istringstream ssin(cmd);
			int id;
			ssin >> ext_cmd >> target_no;
			
			
		}
		if (list[0] == "yell" || list[0] == "name")
		{
			istringstream ssin(cmd);
			ssin >> ext_cmd;
			while(ssin.peek()==' ')ssin.get();
			getline(ssin, chat_msg);
			chat_msg.erase(chat_msg.size()-1,1);
		}
		if (list[0] == "who")
		{
			ext_cmd = list[0];
			
		}
		/*^^^^^^^^^^                     processing extend command                 ^^^^^^^^^^*/
		
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
	int setenv()
	{
		int from = 0;
		if (list[from] == "setenv")
		{
			ENV[list[from+1]] = list[from+2];
			if (list[from+1] != "PATH")return 1;
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
			return 1;
		}
		else return 0;
		
	}
	bool chk_command(int from)
	{
		//cout << "checking " << list[from] << endl;
		if(list[from] == "setenv" || list[from] == "printenv")return 1;
		if(list[from] == "who" || list[from] == "tell" || list[from] == "yell" || list[from] == "name" || list[from] == "change")return 1;
		struct stat statbuf;
		string aim;
		for (int i=1; i<=PATH_size; i++)
		{
			
			aim =ROOT_DIC + PATH[i] + "/" + list[from];

			if (stat(aim.c_str(), &statbuf) == -1) continue;
			else
			{
				return 1;
			}
		}
		return 0;
	}
	string chk_all_cmd()
	{
		// a | b | c
		//-1  1    3   5
		ostringstream sout;
		for (int i = 0; i < pipe_seg.size()-1; i++)
		{
			//cout << "checking " << i << " " << list[pipe_seg[i]+1] << endl;
			if (!chk_command(pipe_seg[i]+1))
			{
				sout << "Unknown command2: [" << list[pipe_seg[i]+1] << "]." << endl;
			}
		}
		return sout.str();
		
	}
	void exec(int from, int to)
	{
		/***************************** processing build_in commands *****************************/
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
			//sleep(11);
			cout << list[from+1] << "=" << ENV[list[from+1]] << endl;
			exit(0);
		}
		
		/***************************** processing build_in commands *****************************/
		
		
		struct stat statbuf;
		success_flag = 0;
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
			cout << "Unknown command: [" << list[from] << "]." << endl;
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
	string my_ip;
	int my_port;
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
		int port = 7001;
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
		cout << "listen: " << r << endl;
		printf("waiting ...\n");
		while(1)
		{
			usleep(500000);
			c_fd = accept(l_fd, (struct sockaddr *) &cin, &len); 
			my_port = ntohs(cin.sin_port);
			char addr_p[INET_ADDRSTRLEN];
			my_ip = inet_ntop(AF_INET, &cin.sin_addr, addr_p, sizeof(addr_p));
			cout << "accept: " << c_fd << endl;
			cout << "IP: " << my_ip << endl;
			cout << "port: " << ntohs(cin.sin_port) << endl;
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
		//if (from == 0 && !Tio.success_flag)return ;
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
