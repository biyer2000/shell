
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include <assert.h>
#include <algorithm> 
#include <regex.h>
#include <vector> 

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN GREAT LESS NEWLINE PIPE GREATGREAT TWOGREAT AMPERSAND GREATAMPERSAND GREATGREATAMPERSAND

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
int yylex();
int times = 0;
void expandwildcardsifnec(std::string *ar);
void expandwildcards(std::string prefix, std::string suffix);
void ins(std::string arg);
int cmp(const void *a, const void* b);
bool cmp (std::string a, std::string b);



%}

%%

goal:
  commands
  ;

commands:
  command
  | commands command
  ;

command: simple_command
      ;


simple_command:	
  pipe_list iomodifier_list background_optional NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

pipe_list:
	pipe_list PIPE command_and_args
	| command_and_args
	;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    //Command::_currentSimpleCommand->insertArgument( $1 );
    
    expandwildcardsifnec($1);
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
    
  }
  ;
iomodifier_list:
  iomodifier_list iomodifier_opt 
  |
  ;

iomodifier_opt:
  GREAT WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    if (Shell::_currentCommand._outFile) {
      printf("Ambiguous output redirect.\n");
      Shell::_currentCommand._valid = false;
    }
    Shell::_currentCommand._outFile = $2;
    
    

  }
  | GREATGREAT WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._append = true;
  }
  | LESS WORD {
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._inFile = $2;
  }
  | TWOGREAT WORD{
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
  }
  | GREATAMPERSAND WORD{
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;
  }
  ;
  | GREATGREATAMPERSAND WORD{
    //printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._append = true;
  }
  ;
  
background_optional:
	AMPERSAND {
		Shell::_currentCommand._background = true;
	}
	|
	;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

//int maxEntries = 20;
//int nEntries = 0;
//char ** entries;
std::vector<std::string> entries;
void ins(std::string arg) {
  Command::_currentSimpleCommand->insertArgument(&arg);
}
bool cmp (std::string a, std::string b) {return a<b;} 



void expandwildcardsifnec(std::string *ar) {
  entries.clear();
  //std::cout <<"expand\n";
  //std::cout << *ar + '\n';
  std::string arg = *ar;
  if (!(arg.find('*') != std::string::npos || arg.find('?') != std::string::npos)) {
    Command::_currentSimpleCommand->insertArgument(ar);
    return;

  } else {
    if (arg.find('?') != std::string::npos) {
      if (arg[arg.find('?') - 1] == '{') {
        *ar = arg;
        Command::_currentSimpleCommand->insertArgument(ar);

        return;
      }
    }
    
    std::string prefix = "";
    expandwildcards(prefix, arg);
    //qsort(entries,nEntries, sizeof(char *), cmp );
    std::sort(entries.begin(), entries.end());
    std::string s = "";
    for (int i = 0; i < entries.size(); i++) {
      std:: string *temp = new std::string();
      *temp = entries[i];
      
      Command::_currentSimpleCommand->insertArgument(temp);
      //free(temp);
    }
    
    if (entries.size() == 0) {
      *ar = arg;
      Command::_currentSimpleCommand->insertArgument(ar);

    }
    return;
    
   
  }  
}

void expandwildcards(std::string prefix, std::string suffix) {
  
  if (suffix.length() == 0) {
    //Command::_currentSimpleCommand.insertArgument(&prefix);
    return;
  }
  //std::string t = "\\";
  //std::cout << t;
  std::string suff = "";
  std::string temp = "";
  if (suffix[0] == '/') {
    suff = suffix.substr(1, suffix.length() - 1);
    if (suff.find('/') != std::string::npos) {
      //temp = suffix.substr(0, suff.find('/') + 2);
      temp = "/";
      //suffix = suff.substr(suff.find('/') + 1, suff.length() - suff.find('/') - 1);
      suffix = suff;

    } else {
      temp = suffix;
      suffix = "";
    }
    
  } else {

    if (suffix.find('/') != std::string::npos) {
      temp = suffix.substr(0, suffix.find('/') + 1);
      suffix = suffix.substr(suffix.find('/') + 1, suffix.length() - suffix.find('/') - 1);

    } else {
      temp = suffix;
      suffix = "";
    }
  }
  std::string newpref = "";
  if (!(temp.find('*') != std::string::npos || temp.find('?') != std::string::npos)) {
    newpref += prefix;
    newpref += temp;
    expandwildcards(newpref, suffix);
    return;
  }
  std::string reg = "";
  int i = 0;
  reg = "^";
  while (i < temp.length()) {
    if (temp[i] == '*') {
      reg += '.';
      reg += '*';
    } else if (temp[i] == '?') {
      reg += '.';
    } else if (temp[i] == '.') {
      reg += '\\';
      reg += '.';
    } else if (temp[i] == '/' && i == temp.length() - 1) {
      reg += "";

    } else {
      reg += temp[i];
    }
    i++;
  }
  reg += '$';
  
  regex_t re;
  char* regx = const_cast<char*>(reg.c_str());
  int exbuf = regcomp(&re, regx, REG_EXTENDED|REG_NOSUB);
  if (exbuf != 0) {
    perror("regomp");
    regfree(&re);
    exit(1);
  }
  char *dir;
  std::string dire = "";
  if (prefix.length() == 0) {
    //dir = strdup(".");
    dire += ".";
    dir = const_cast<char*>(dire.c_str());
  } else {
    dir = const_cast<char*>(prefix.c_str());
  }
  DIR *d = opendir(dir);
  //free(dir);
  if (d == NULL) {
    regfree(&re);
    //free(dir);
    return;
  }
  struct dirent *ent;
  regmatch_t match;
  bool ins = false;
  while ((ent = readdir(d)) != NULL) {
    ins = false;
    if (regexec(&re, ent->d_name,1, &match, 0) == 0) {
      newpref = "";
      newpref += prefix;
      if (ent->d_name[0] == '.') {
        if (temp[0] == '.') {
          ins = true;
          newpref += ent->d_name;
          if (temp[temp.length() - 1] == '/' && prefix[prefix.length() - 1] == '/') {
            newpref += '/';
          }
          expandwildcards(newpref, suffix);
        }
      } else {
        //newpref += prefix;
        ins = true;
        newpref += ent->d_name;
        if (temp[temp.length() - 1] == '/' && prefix[prefix.length() - 1] == '/') {
            newpref += '/';
          }
        expandwildcards(newpref, suffix);
      }
      if (suffix.length() == 0) {
        if (ins) {
          std::string entry = prefix;
          char *temp = strdup(ent->d_name);
          entry += temp;
          free(temp);
          
          entries.push_back(entry);
          
        }
        
      }
    }
  }
  regfree(&re);
  closedir(d);
  
}






#if 0
main()
{
  yyparse();
}
#endif


