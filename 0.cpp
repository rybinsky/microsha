#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <fnmatch.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <utility>
#include <string.h>
#include <iostream>
#include <fstream>
#include "algorithm_dfa.cpp"

using namespace std;
//=====================================================//
const vector<char> Sigma {
    'q','w','e','r','t','y','u','i','o','p','a','s','d','f','g','h','j','k','l','z','x',
    'c','v','b','n','m','.','Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L','Z','X',
    'C','V','B','N','M' ,'0','1','2','3','4','5','6','7','8','9','-'
};
//=====================================================//
string pumping(const string& str);
bool checking(const string& str, const string& path);
template <typename T>
vector<T> Unification(const vector<T>& m1,const vector<T>& m2);
vector<string> Way_to_ways(const string& path, const string& expr);
vector<string> Possible_ways(const vector<string>& mass);
vector<string> Tokenizator(string a);
void Parse_Line(string& line, vector<string>& ans);
vector<string> Ways(vector<string> args);
int Execution(vector<string> &args);
void InOutStreams(vector<string> &args);
void Piping(const vector< vector<char *>> &argsCharVector, int NumberCmd, int CountCmds, int parentPid);
int loop();
//======================================================//
int main(int argc, char** argv) {
	loop();
	return 0;
}
//=====================================================//
//main loop
int loop() {
	string Line;
	
	while(1) {
		pid_t uid = getuid();
		char curDir[1000];
		
		getcwd(curDir, 1000 * sizeof(char));
		if (uid == 0) { cout << "[" << curDir << "]" << "! "; }
		else { cout << "[" <<  curDir << "]" << "> "; }
		
		getline(cin, Line);
		if (cin.eof() && (Line.length() == 0)) { 
			cout << endl;
			break;
		}
		if (cin.eof()) return 0;
		if (Line.size() == 0) continue;

		vector<string> args;
		Parse_Line(Line, args);
		if (args.size() > 0 && args[0] == "exit") {
			return 0;
		}	
		
		args = Ways(args);

		Execution(args);
	}
	return 0;
}
//========================================================//
int Execution(vector<string> &args) {
	int pid = fork();
		
		if (pid < 0) perror("fork");
		if (pid == 0) { //child
			InOutStreams(args);
		} else {
			wait(0);
		}

		if (pid == 0) {
			if ((args.size() > 0) && (args[0] == "time")) {
				struct rusage r_usage, rr_usage;
				struct timeval sys_start, sys_end, u_start, u_end, 
											real_start, real_end;
				vector<char *> SingleCmd;
				for (int j = 1; j < args.size(); j++) {
					SingleCmd.push_back((char *)args[j].c_str());
				}
				SingleCmd.push_back(NULL);
				pid_t parent_pid = getpid();
				pid_t newpid = fork();
				if (newpid < 0) perror("fork");
				if (newpid == 0) {
					if (execvp(SingleCmd[0], &SingleCmd[0]) < 0) {
						perror("execvp");
						_exit(0);
					}
				} else {
					gettimeofday(&real_start, NULL);
					getrusage(RUSAGE_CHILDREN, &r_usage);
					wait(0); 
					getrusage(RUSAGE_CHILDREN, &rr_usage);
					gettimeofday(&real_end, NULL);
					sys_start = r_usage.ru_stime;
					u_start = r_usage.ru_utime;
					sys_end = rr_usage.ru_stime;
					u_end = rr_usage.ru_utime;
					long long sys_sec = sys_end.tv_sec - sys_start.tv_sec;
					long long sys_usec = (sys_end.tv_usec - sys_start.tv_usec) / 1000;
					long long u_sec = u_end.tv_sec - u_start.tv_sec;
					long long u_usec = (u_end.tv_usec - u_start.tv_usec) / 1000;
					long long real_sec = real_end.tv_sec - real_start.tv_sec;
					long long real_usec = (real_end.tv_usec - real_start.tv_usec) / 1000;
					cout << endl;
					printf("real	  [%ld.%03lds]\n", real_sec, real_usec);
					printf("user      [%ld.%03lds]\n", u_sec, u_usec);
					printf("sys       [%ld.%03lds]\n", sys_sec, sys_usec);
				}
				_exit(0);
			}
		} else {
			wait(0);
		}
		//cd 	
		if (args[0] == "cd") {
			if(args.size() == 1){
                fprintf(stderr, "Expected argument to \"cd\"\n");
            }
            else{
                chdir(args[1].c_str());
            }
			return 0;
		}

		if (pid == 0) { // Child	
			if (args.size() == 0) _exit(0);
			if (args[0] == "|") _exit(0);
			if (args[args.size() - 1] == "|") _exit(0);
			int i = 0;
			int lastIndex = -1;
			int CountCmds = 1;
			vector<vector<char*>> argsCharVector;
			while (i < args.size()) {
				if ((args[i] != "|") && (i+1 < args.size())) {
					i++;
					continue;
				}
				if (i+1 == args.size()) i++;
				vector<char*> SingleCmd;
				for (int j = lastIndex+1; j < i; j++) { // pipe vector
					SingleCmd.push_back((char *)args[j].c_str());
				}
				lastIndex = i;
				SingleCmd.push_back(NULL);
				argsCharVector.push_back(SingleCmd);

				if (i != args.size()) CountCmds++;
				i++;
			}
			
			if (CountCmds > 0) {
				Piping(argsCharVector, 1, CountCmds, pid);
			}
		} else { // Parent
			wait(0);
		}
	return 0;
}
//=====================================================//
vector<string> Ways(vector<string> args){
	vector<string> temp_args;
	for(int i = 0; i < args.size(); i++){
		if (args[i].find("*") != args[i].npos || args[i].find("?") != args[i].npos){

			vector<string> folders = Tokenizator(args[i]);
			vector<string> ans;
			ans = Possible_ways(folders);
			
			for(int j=0; j < i; j++){
				temp_args.push_back(args[j]);
			}
			for(auto i : ans){
				temp_args.push_back(i);
			}
			for(int j = i + 1; j < args.size(); j++){
				temp_args.push_back(args[j]);
			}		

		} else {		
			temp_args.push_back(args[i]);
		}
	}
	return temp_args;
}
//=====================================================//
void Parse_Line(string& str, vector<string>& ans) {
	str = ' ' + str;
	int i = 1;
	while ((i < str.length()) && ((str[i] == ' ') || (str[i] == '\t'))) i++; 
	while (i < str.length()) {
		int start = i;
		while (1) {
			if ((i < str.length()) && (str[i] != ' ') && (str[i] != '\t')) {
				i++;
				continue;
			} else {
				break;
			}
		}
		ans.push_back(str.substr(start, i - start));
		while ((i < str.length()) && ((str[i] == ' ') || (str[i] == '\t'))) i++;
	}
	return; 
}
//=====================================================//
vector<string> Tokenizator(string a) {
	vector<string> tokens;
	string temp;
	while (!a.empty()) {
		if (a.rfind("/") != a.npos) {
			auto it = a.rfind("/");
			temp = a.substr(it + 1);
			if (!temp.empty()) tokens.push_back(temp);
			tokens.push_back("/");

			a = a.substr(0, it);
		}
		else {
			tokens.push_back(a);
			break;
		}
	}
	reverse(tokens.begin(), tokens.end());
	return tokens;
}
//=====================================================//
void InOutStreams(vector<string> &args) {
	int i = 0;
	bool Changed = false;

	while (i < args.size()) { 
		Changed = false;

		if ((args[i].find('<') != args[i].npos) || 
			(args[i].find('>') != args[i].npos)) {
			Changed = true;
			int pos = (args[i].find('>') == args[i].npos) ? args[i].find('<') : args[i].find('>');
			bool isOutput = (args[i].find('>') == args[i].npos) ? false : true;
			int j = i;
			string substr1 = args[j].substr(0, pos);
			string substr2 = args[j].substr(pos + 1, args[j].length() - pos - 1);
			
			bool substr3exists = (j == args.size() - 1) ? false : true;
			string substr3 = "";
			if (substr3exists) substr3 = args[j + 1];
			//cout << "i = " << i << ": " << substr1 << " " << substr2 << " " << substr3 << endl;
			if (((substr1 != "0") && (substr1 != "") && (!isOutput)) || ((substr1 != "1") && (substr1 != "2") && (substr1 != "") && (isOutput))) {
				args.insert(args.begin() + j, substr1);
				substr1 = "";
				j++; //
			}
			if (substr1 == "") {
				substr1 = "0";
				if (isOutput) substr1 = "1";
			}  
		
			if (substr2 == "") {
				substr2 = substr3;
				args.erase(args.begin() + j + 1);
			}
			args.erase(args.begin()+j);
			
			close(stoi(substr1));
			int redirectTo = -1;
			if (isOutput) {
				redirectTo = open(substr2.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
			} else {
				redirectTo = open(substr2.c_str(), O_RDONLY, 0666);
			}
			if (redirectTo < 0) perror("open");
			break;
		}
		i++;
	}
	if (Changed) InOutStreams(args);
}
void Piping(const vector<vector<char*>>& argsCharVector, int NumberCmd, int CountCmds, int parentPid) {
	if (NumberCmd == CountCmds) { 
		if (execvp(argsCharVector[CountCmds - NumberCmd][0], &argsCharVector[CountCmds - NumberCmd][0]) < 0) { //1st command
			write(2, argsCharVector[CountCmds - NumberCmd][0], strlen(argsCharVector[CountCmds - NumberCmd][0]));
			string err = ": command not found\n";
			write(2, err.c_str(), err.length());
			//perror("execvp");
			_exit(0);
		}
		return;		
	}
	int fd[2];
	if (pipe(fd) < 0) perror("pipe");
	pid_t newpid = fork();
	if (newpid < 0) perror("fork");
	if (newpid == 0) {
		close(1);
		int fd_1 = dup(fd[1]);
		Piping(argsCharVector, NumberCmd+1, CountCmds, newpid);
	} else {
		close(0);
		close(fd[1]);
		int fd_0 = dup(fd[0]);
		if (execvp(argsCharVector[CountCmds - NumberCmd][0], &argsCharVector[CountCmds - NumberCmd][0]) < 0) {
			write(2, argsCharVector[CountCmds - NumberCmd][0], strlen(argsCharVector[CountCmds - NumberCmd][0]));
			string err = ": command not found\n";
			write(2, err.c_str(), err.length());
			//perror("execvp");
			_exit(0);
		}
	}
	return;
}
//=====================================================//
vector<string> Possible_ways(const vector<string>& mass) {

	vector<string> ways;

	if(mass[0] == "/") {
		ways.push_back("/");
	} else  {
		ways.push_back(".");
	}

	for(int i = 0 ; i < mass.size() ; i++) {
		vector<string> ways_temp;
		vector<string> temp;
		string expr = pumping(mass[i]);
		
		if( mass[i] != "/" && mass[i] != "." && mass[i] != "*") {
			for(int j = 0 ; j < ways.size() ;++j) {
				temp = Way_to_ways(ways[j], expr);
				ways_temp = Unification(ways_temp, temp);
			}
			ways = ways_temp;
		}else if (mass[i] == "*") {
			vector<string> temp;
			for(auto i : ways){
                DIR *dir;
                struct dirent *de;
                dir = opendir(i.c_str());
                while(dir){
                    de = readdir(dir);
                    if (!de) break;
                    if (de->d_name[0] != '.' && de->d_type == 4) {
                        temp.push_back(i + '/' + (string)de->d_name);
                    }else if (de->d_name[0] != '.' && de->d_type == 8) {
						temp.push_back(i + '/' + (string)de->d_name);
					}
                }
                closedir(dir);                
            }
			ways = temp;
		}
    }
   return ways;	
}
//=====================================================//
string pumping(const string& str) {
    string temp;
    for(int i = 0; i < str.size(); i++) {
        if(str[i] == '*') {
            temp += "(";
            for(int i = 0 ; i < Sigma.size() - 1 ; i++)  
				temp = temp + Sigma[i] + "|";
            temp += Sigma[Sigma.size() - 1];
            temp += ")*";
        } else if(str[i] == '?') {
            temp += "(";
            for(int i = 0; i < Sigma.size() - 1; i++)
				temp = temp + Sigma[i] + "|";
            temp += Sigma[Sigma.size() - 1];
            temp += ")";
        } else {
            temp += str[i];
        }
    }
	return temp;
}
//=====================================================//
vector<string> Way_to_ways(const string& path, const string& expr) {
    
    vector<string> res;

    DIR *dir;
    struct dirent *de;
    dir = opendir(path.c_str()); /*your directory*/
	while(dir) {
        de = readdir(dir);
        if (!de) break;
		if(de->d_name[0] != '.' && checking(expr, de->d_name)) {
          if(path == "/") {;
		    res.push_back(path +(string)de->d_name);
		  } else {
				res.push_back(path + "/" +(string)de->d_name);
		  }
        }
    }
    closedir(dir);
	return res;
}
//=====================================================//
template <typename T>
vector<T> Unification(const vector<T>& m1,const vector<T>& m2) {
  vector<T> ans;
  for(const auto& i : m1){
	  ans.push_back(i);
  }
  for(const auto& i : m2){
	  ans.push_back(i);
  }
  return ans;
}
//=====================================================//
bool checking(const string& str, const string& path) {
    re_dfa_type dfa;
    bool ok = dfa.compile(str.cbegin(), str.cend());
    int current = 0;
    for(int i = 0; i < path.size(); i++) {
        map<int, int> k = dfa.m_transition[current];
		if (k.count((int)path[i])){
			current = k[(int)path[i]];
		} else {
			return 0;
		}
    }
    if((dfa.m_transition[current])[-1] == -1) return 1;
    return 0;
}