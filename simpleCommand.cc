#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "common.hh"
//#include "shell.cc"
#include <string.h>


#include "simpleCommand.hh"
//extern std::string argu = "";

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}
std::string SimpleCommand::tilde(std::string arg) {
  std::string temp = "";
  if (arg.length() == 1) {
    temp = getenv("HOME");

  } else {
    //std::cout << arg + '\n';
    //std::cout << arg.substr(1, arg.length() - 1) + '\n';
    if (arg.find('/') != std::string::npos) {
      temp = getpwnam(arg.substr(1, arg.find('/') - 1).c_str())->pw_dir;
      temp += '/';
      temp += arg.substr(arg.find('/') + 1, arg.length() - arg.find('/') - 1);

    } else {
      temp = getpwnam(arg.substr(1, arg.length() - 1).c_str())->pw_dir;

    }
    
    //std::cout << "directed\n";
    //std::cout << temp + '\n';

  }
  return temp;
}
std::string SimpleCommand::geten(std::string arg) {
  std::string temp = "";
  //std::cout << arg + '\n';
  std::string t = "";
  char *env = NULL;
  int pid = 0;
  for (size_t i = 0; i < arg.length(); i++) {
    if (i < arg.length() - 3 && arg[i] == '$' && arg[i + 1] == '{') {
      t = "";
      size_t j = 0;
      for (j = i + 2; j <= arg.length() - 2; j++) {
        t += arg[j];
        if (arg[j + 1] == '}') {
          break;
        }

      }
      const char * par = NULL;
      
      if (t.compare("$") == 0|| t.compare("?") == 0 || t.compare("!") == 0 || t.compare("_") == 0 || t.compare("SHELL") == 0) {
        if (t.compare("$") == 0){
          temp += std::to_string(getpid());
        } 
        if (t.compare("SHELL") == 0) {
          char *buffer = (char*)(malloc(4096));
          
          temp += realpath("../shell", buffer);
          free(buffer);
        } else if (t.compare("!") == 0) {
          temp += getenv("!");
        } else if (t.compare("?") == 0) {
          temp += getenv("?");

        } else if (t.compare("_") == 0) {
          //std::cout << "uscore\n";
          temp += getenv("_");
          //std::cout << temp + '\n';
        }

      } else {
        par = t.c_str();
        

        
        env = getenv(par);
      }
      
      
      if (env != NULL) {
        std::string te = env;
        temp += te;
        i = j + 1;
      } else {
        i = j + 1;
      }
    
      
      
      if (i >= arg.length()) {
        
        break;
      }
    } else {
      temp += arg[i];
    }
  }
  return temp;
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  //std::cout << *argument + '\n';
  std::string temp = *argument;
  if (temp[0] == '~') {
    *argument = tilde(*argument);
  } else {
    *argument = geten(*argument);

  }
  //std::cout << "pushed";
  
  
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}
