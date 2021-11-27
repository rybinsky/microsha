#include <unistd.h>     
#include <sys/types.h>  
#include <sys/wait.h>   
#include <signal.h>     
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <sys/stat.h> 
#include <fcntl.h> 
#include <utility>
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <utility>
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
vector<pair<string, int>> Way_to_ways(const string& path, const string& expr);
vector<pair<string, int>> Possible_ways(const vector<string>& mass);
vector<string> Char_to_Str(char** args);
vector<string> Tokenizator(string a);
void Ls_exec(const vector<string>& argums);
int LS(char** args);
pair<char**, int> Parse_Line(char* line);
void Piper(char *cmd,char* args[], int argnum, int& Stop);
void Execution(char* cmd, char* args[], int argnum, int& Stop);
void loop();
//======================================================//
int main(int argc, char** argv) {
	loop();
	return 0;
}
//=====================================================//
//главный цикл
void loop() {
	char** args;
	int CountArgs;
	string Line;
	char* line;
	int Stop = 0;

	while(!Stop) {
		int p = 0;
		pid_t uid = getuid();
		if (p == 0) {
			char str[100];
			getcwd(str, 100 * sizeof(char));
			if (uid == 0) {
				cout << "[" << str << "]" << "! ";
			}
			else {
				cout << "[" << str << "]" << "> ";
			}
		}
		if(!getline(cin,Line) ) { Stop = 1; }
		if (Line.size() == 0) continue;
		line = (char*)malloc((Line.length() + 1) * sizeof(char));
		//cout<<"line"<<" "<<(Line.length() + 1)<<"bytes"<<' '<<(void*)line<<endl;//выделил память под строку line
        strcpy(line, Line.c_str());

		//pair<char**, int> Tokens = Parse_Line(line);
		//args = Tokens.first;
		//CountArgs = Tokens.second;
		string buf; 
		stringstream ss(line);
		vector<char*> tokens;
		char *temp;

		while (ss >> buf) {
			temp = (char*)malloc((buf.length() + 1) * sizeof(char));
			//cout<<"temp"<<" "<<(buf.length() + 1)<<"bytes"<<' '<<(void*)temp<<endl;
			//выделяю память под один токен, всегда разной длины, тк buf разный
			strcpy(temp,buf.c_str());
			tokens.push_back(temp);
			//cout<<temp<<' '<<&temp<<endl;
		}
		char** argv = (char	**)malloc((tokens.size() + 1) * sizeof(char*));
		//cout<<"argv"<<" "<<(tokens.size() + 1)<<"bytes"<<' '<<(void*)argv<<endl;
		//выделил память t.size()+1 под массив указателей
		for (int i = 0; i < tokens.size(); i++ ) {
			argv[i] = tokens[i];
			//cout<<argv[i]<< ' '<<&argv[i]<<endl;
			//cout<<tokens[i]<<' '<<&tokens[i]<<endl;
		}
		//cout<< temp << &temp << endl;
		argv[tokens.size()] = NULL;//последний t.size + 1-й элемент на NULL
		//cout<<"last NULL address "<< (void*)argv[tokens.size()]<<endl;
		CountArgs = tokens.size();
		//cout<<"CountArgs="<<CountArgs<<endl;
		args = argv;
		//cout<<temp<<' '<<&temp<<endl;
		Execution(args[0],args, CountArgs, Stop);
		//cout<<"Success"<<endl;
		//cout<<line<<' ';
		free(line);//очистил строку lline
		//cout<<(void*)line<<endl;
		//cout<<"Line empty"<< endl;
		for(int i = 0; i < CountArgs + 1; i++) {
			//cout<<"args[i]"<<' '<<(void*)args[i]<<endl;
			if (args[i] != NULL) free(args[i]);//очищаю по кажд указ из массива
		}//очистил t.size элементов, если пропиу еще +1, то ничего не изм-ся
		//cout<<"argv"<<' '<<(void*)argv<<endl;
		free(args);//очистил массив указателей
		//cout<<"Args empty"<<endl;
	}
}
//=====================================================//
pair<char**, int> Parse_Line(char* line) {
    string buf; 
    stringstream ss(line);
    vector<char*> tokens;
	char *temp;

    while (ss >> buf) {
		temp = (char*)malloc((buf.length() + 1) * sizeof(char));
        strcpy(temp,buf.c_str());
        tokens.push_back(temp);
		//cout<<temp<<' '<<&temp<<endl;
    }
    char** argv = (char**)malloc((tokens.size() + 1) * sizeof(char*));
    for (int i = 0; i < tokens.size(); i++ ) {
      	argv[i] = tokens[i];
		//cout<<argv[i]<< ' '<<&argv[i]<<endl;
		//cout<<tokens[i]<<' '<<&tokens[i]<<endl;
    }
	//cout<< temp << &temp << endl;
	argv[tokens.size()] = NULL;
	return {argv, tokens.size()};
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
vector<string> Char_to_Str(char** args) {
	vector<string> ans;
	while (*args != NULL) {
		ans.push_back(*args);
		args++;
	}
	return ans;
}
//=====================================================//
void Execution(char* cmd, char* args[], int argnum, int& Stop) {
	int pipes = 0;

    for(int i = 0; i < argnum; i++){
		if(strcmp(args[i], "|") == 0) { pipes = 1; }
    }

	if (pipes) {
		//cout<<"piper in Exec"<<endl;
		for(int i = 0; i < argnum; i++) {
			if(strcmp(args[i], "|") == 0) {
				//free(args[i]);
               	//args[i] = NULL;
                char* right[argnum - i];
                int c = 0;
                for(int j = i; j < argnum - 1; j++) {
	                right[j - i] = args[j + 1];
					//cout<< right[j - i]<<endl;
                    c++;
                }
                right[c] = NULL;
                int p[2];
                pipe(p);
                pid_t lpid = fork();
                if(lpid == 0){//left (child)
					dup2(2,1);
                    dup2(p[1], STDOUT_FILENO);
					//cout<<"0000000"<<endl;
                    execvp(cmd, args);
                }
                else {//right (parent)
					pid_t rpid = fork();
					close(p[1]);
					if(rpid == 0){ //right child
						dup2(2,1);
                    	dup2(p[0], STDIN_FILENO);

                    	Piper(right[0], right, c, Stop);
						close(p[1]);
					} else { //parent
						waitpid(rpid, 0, 0);
					}
                }
                break;
			}
		}
    }
	else{
        if(strcmp(cmd, "cd") == 0) {
            if(args[1] == NULL){
				chdir("/");
                //fprintf(stderr, "Expected argument to \"cd\"\n");
            }
            else{
                chdir(args[1]);
            }
        }
		else 
		if(strcmp(cmd, "ls") == 0) {
            LS(args);
        }
        else { //fork
      		pid_t childID = fork();
      		if(childID < 0) {
        		perror("Error when forked");
				Stop = 1;
      		}
      		else 
			if(childID == 0) { //Child process
                for(int i = 0; i < argnum; i++) {
                    if(strcmp(args[i], ">") == 0) {
                        int newstdout = open(args[i + 1], O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                        close(1);
                        dup(newstdout);
                        close(newstdout);
						//free(args[i]);
                        //args[i] = NULL;
                    }
                    else if(strcmp(args[i], "<") == 0){
						//cout<<")))<((("<<endl;
                        int newstdin = open(args[i+1], O_RDONLY);
						//cout<<"((()))"<<endl;
                        close(0);
                    	dup(newstdin);
                    	close(newstdin);
						//free(args[i]);
                        //args[i] = NULL;
                    }
                }
        		execvp(cmd, args);
				//cout<<"Perrorrrrrrrrrrrrrrrrrrrrrrrrrr"<<endl;
        		perror(cmd); //when there is an error
				Stop = 1;
      		}
			else {
				if (waitpid(childID, 0, 0) < 0) {//parent process
          			Stop = 1;
	  				perror("Error when waiting for child");
        		}
      	  	}
    	  }
    }
}
//=====================================================//
void Piper(char *cmd,char* args[], int argnum, int& Stop) {
    int pipes = 0;
	//cout<<"PIPER"<<endl;
    for(int i = 0; i < argnum; i++){
        if(strcmp(args[i], "|") == 0) {pipes = 1; }
    }
    
	if(pipes) {
        for(int i = 0; i < argnum; i++){
                if(strcmp(args[i], "|") == 0){
						//free(args[i]);
                        //args[i] = NULL;
                        char* right[argnum - i];
                        int c = 0;
                        for(int j = i; j < argnum - 1; j++){
                                right[j - i] = args[j + 1];
								//cout<< right[j - i]<<endl;
                                c++;
                        }
                        right[c] = NULL;
                        int p[2];
                        pipe(p);
                        pid_t lpid = fork();
                        if(lpid == 0) {//left (child)
                            dup2(2, 1);
                            dup2(p[1], STDOUT_FILENO);
                            execvp(cmd, args);
                        }
                        else {//right (parent)
                            pid_t rpid = fork();
                            close(p[1]);
                            if(rpid == 0) { //right child
                                dup2(2, 1);
                                dup2(p[0], STDIN_FILENO);
                                Piper(right[0], right, c, Stop);
                                close(p[1]);
                            } else { //parent
								waitpid(rpid, 0, 0);
							}
						}
                        break;
                }
        }
    }
    else {
        if(strcmp(cmd, "cd") == 0){
            if(args[1] == NULL){
                fprintf(stderr, "Expected argument to \"cd\"\n");
            }
            else{
                chdir(args[1]);
            }
        }
		else 
		if(strcmp(cmd, "ls") == 0) {
            LS(args);
        }
        else{ //forking
            pid_t childID = fork();
            if(childID < 0){
                perror("Error when forked");  
				Stop = 1;               
            }
            else 
			if(childID == 0){ //Child process
                        for(int i = 0; i < argnum; i++){
                                if(strcmp(args[i], ">") == 0){
                                        int newstdout = open(args[i + 1], O_WRONLY | O_CREAT , S_IRWXU | S_IRWXG | S_IRWXO);
                                        close(1);
                                        dup(newstdout);
                                        close(newstdout);
										//free(args[i]);
                                        //args[i] = NULL;
                                }
                                else if(strcmp(args[i], "<") == 0){
										//cout<<")))<((("<<endl;
                                        int newstdin = open(args[i + 1], O_RDONLY);
                                        close(0);
                                        dup(newstdin);
                                        close(newstdin);
										//free(args[i]);
                                        //args[i] = NULL;
                                }
                        }
                        execvp(cmd, args);
						//cout<<"Perrorrrrrrrrrrrrrrrrrrrrrrrrrr"<<endl;
                        perror(cmd); //when there is an error
						Stop = 1;
            }
			else{
				if (waitpid(childID, 0, 0) < 0) {//parent process
          			Stop = 1;
	  				perror("Error when waiting for child");
        		}
      	  	}
            
        }
    }
	//cout<<"End PIPER"<<endl;
	Stop = 1;
}
//=====================================================//
int LS(char** args) {

	vector<string> argums = Char_to_Str(args);

	if (argums.size() == 1 || argums.back()[0] == '-') {
		Ls_exec(argums);
		return 1;
	}
	else {
		vector<string> folders = Tokenizator(argums.back());
		vector<pair<string, int>> ans;
		ans = Possible_ways(folders);

		for(auto i : ans ) {
			if (i.second == 0) {
				argums.back() = i.first;
				Ls_exec(argums);
			}
		}
		for(auto i : ans) {
			if (i.second == 1) {
				argums.back() = i.first;	
				cout << argums.back() << ':' << endl;	
				Ls_exec(argums);
			} else {
				continue;
			}
		}
	}
	return 1;
}
//=====================================================//
void Ls_exec(const vector<string>& argums) {
	vector<char*> args0; 
	for (size_t i = 0; i < argums.size(); ++i) {
		args0.push_back((char*)argums[i].c_str());
	}
	args0.push_back(nullptr);
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		execvp(args0[0], &args0[0]);
		perror(args0[0]);
		exit(0);
	}
	else if (pid > 0) {
		wait(0);
		}
}
//=====================================================//
vector<pair<string, int>> Possible_ways(const vector<string>& mass) {

	vector<pair<string, int>> ways;

	if(mass[0] == "/") {
		ways.push_back({"/", 0});
	} else  {
		ways.push_back({".", 0});
	}

	for(int i = 0 ; i < mass.size() ; i++) {
		vector<pair<string, int>> ways_temp;
		vector<pair<string, int>> temp;
		string expr = pumping(mass[i]);
		
		if( mass[i] != "/" && mass[i] != "." && mass[i] != "*") {
			for(int j = 0 ; j < ways.size() ;++j) {
				temp = Way_to_ways(ways[j].first, expr);
				ways_temp = Unification(ways_temp, temp);
			}
			ways = ways_temp;
		}else if (mass[i] == "*") {
			vector<pair<string, int>> temp;
			for(auto i : ways){
                DIR *dir;
                struct dirent *de;
                dir = opendir(i.first.c_str());
                while(dir){
                    de = readdir(dir);
                    if (!de) break;
                    if (de->d_name[0] != '.' && de->d_type == 4) {
                        temp.push_back({i.first + '/' + (string)de->d_name, 1});
                    }else if (de->d_name[0] != '.' && de->d_type == 8) {
						temp.push_back({i.first + '/' + (string)de->d_name, 0});
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
vector<pair<string, int>> Way_to_ways(const string& path, const string& expr) {
    
    vector<pair<string, int>> res;

    DIR *dir;
    struct dirent *de;
    dir = opendir(path.c_str()); /*your directory*/
	while(dir) {
        de = readdir(dir);
        if (!de) break;
		if(de->d_name[0] != '.' && checking(expr, de->d_name)) {
          if(path == "/") {;
		    res.push_back({path +(string)de->d_name, 0});
		  } else {
            if (de->d_type == 4) 
				res.push_back({path + "/" +(string)de->d_name, 0});
			else
				res.push_back({path + "/" +(string)de->d_name, 1});
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