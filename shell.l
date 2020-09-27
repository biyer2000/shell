
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "y.tab.hh"
#include <iostream>
#include <unistd.h>




extern "C" char * read_line();

int mygetc(FILE * f) {
  static char *p;
  char ch;

  if (!isatty(0)) {
  // stdin is not a tty. Call real getc
    return getc(f);
  }

  // stdin is a tty. Call our read_line.
  if (p==NULL || *p == 0) {
    char * s = read_line();
    std::cout << s;
    std::cout << "\n";
    p = s;
  }

  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)













static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}
#define MAX_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_DEPTH];
int include_stack_ptr = 0;
%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}
">>" {
  return GREATGREAT;
}
"2>" {
  return TWOGREAT;
}
"|" {
  return PIPE;
}
"<" {
  return LESS;
}
"&" {
  return AMPERSAND;
}
">&" {
  return GREATAMPERSAND;
}
">>&" {
  return GREATGREATAMPERSAND;
}
\$\([^\n\$]*\)|`[^\n]*` {
  //subshell
  std::string comm = std::string(yytext);
  //std::cout << comm + '\n';
  if (comm.at(0) == '$') { 
    //($(..))
    comm = comm.substr(2, comm.length() - 3);

  } else {
    //`..`
    comm = comm.substr(1, comm.length() - 2);
  }
  //std::cout << comm + '\n';
  int tempin = dup(0);
  int tempout = dup(1);
  //std::cout << "pin\n";
  int pin[2];
  int pout[2];
  //std::cout << "pin\n";
  pipe(pin);
  pipe(pout);
 //std::cout << "fork";
  write(pin[1], comm.c_str(), comm.length());
  write(pin[1], "\n", 1);
  write(pin[1], "exit\n", 5);
  
  close(pin[1]);
  dup2(pin[0], 0);
  close(pin[0]);
  dup2(pout[1], 1);
  close(pout[1]);
  //std::cout << "fork";
  int ret = fork();
  if (ret == 0) {
     execvp("/proc/self/exe", NULL);
     exit(1);
  } else if (ret < 0) {
    perror("fork");
    exit(1);
  }
  dup2(tempin, 0);
  dup2(tempout, 1);
  close(tempin);
  close(tempout);

  char c = NULL;
  char buffer[4096];
  int in = 0;
  while (read(pout[0],&c, 1)) {
    if (c == '\n') {
      buffer[in] = ' ';
      in++;
    } else {
      buffer[in] = c;
      in++;
    }

  }
  buffer[in] = '\0';
  close(pout[0]);

  for (int i = strlen(buffer) - 1; i >= 0; i--) {
    myunputc(buffer[i]);
  }
  


}
"source "[^ \t\n]* {
  char *string = new char(strlen(yytext) - 6);
  strcpy(string, yytext + 7);
  string[strlen(yytext) - 7] = '\0';
  if ( include_stack_ptr >= MAX_DEPTH ) {
    fprintf( stderr, "Includes nested too deeply" );
    exit( 1 );
  }
  include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
  yyin = fopen(string, "r");
  delete(string);
  if ( ! yyin )
    perror("Error" );
    yy_switch_to_buffer(
    yy_create_buffer( yyin, YY_BUF_SIZE ) );
    BEGIN(INITIAL);
}

<<EOF>> {
  if ( --include_stack_ptr < 0 ) {
    yyterminate();
  }
  else {
    yy_delete_buffer( YY_CURRENT_BUFFER );
    yy_switch_to_buffer(
    include_stack[include_stack_ptr] );
  }        
}

[^ \t\n<>|&]*\\[^ \t\n]* {
  //gets escape character
  std::string s = yytext;
  //std::cout << s + '\n';
  std::string temp = "";
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == '\\' && i != s.length() - 1) {
      temp += s[i + 1];
      i++;
    } else  if (s[i] != '\\') {
      temp += s[i];
    }
  }
  yylval.cpp_string = new std::string(temp);
  return WORD;
}

\"[^\n]*\" {
  std::string s = std::string(yytext);
  yylval.cpp_string = new std::string(s.substr(1, s.length() - 2));
  return WORD;
}




[^ \|\t\n\<\>\\\&][^ \|\t\n\<\>\\\&]* {
  /* Assume that file names have only alpha chars */
  //std::cout << std::string(yytext);
  yylval.cpp_string = new std::string(yytext);
  //std::cout << yylval.cpp_string + '\n';
  return WORD;
}
