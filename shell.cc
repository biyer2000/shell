#include <cstdio>
#include <signal.h>

#include "shell.hh"
#include <sys/wait.h>
#include <unistd.h>


int yyparse(void);

void Shell::prompt() {
  if (isatty(0)) {
    printf("myshell>");
  }
  fflush(stdout);
  
  
}


extern "C" void ctrlc( int sig )
{
	if (sig == SIGINT) {
    printf("\n");
    printf("exiting\n");
    exit(EXIT_SUCCESS);
  }
}

extern "C" void zombie( int sig )
{
    if (sig == SIGCHLD) {
      int pid = wait3(0, 0, NULL);
      while (waitpid(-1, NULL, WNOHANG) > 0); 
        //printf("\n");
      if (pid != -1 ) {
        if (isatty(0)) {
          //printf("[%d] exited\n", pid );
          //Shell::prompt();
        }
      }
    }
      

    
	
}



int main() {
  
  struct sigaction sa;
  sa.sa_handler = ctrlc;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sa, NULL)){
      perror("sigaction");
      exit(EXIT_FAILURE);
  }
  
  struct sigaction sa2;
  sa2.sa_handler = zombie;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = SA_RESTART;

  if(sigaction(SIGCHLD, &sa2, NULL)){
      perror("sigaction");
      exit(EXIT_FAILURE);
  }
  
  
  Shell::prompt();
  yyparse();
  

}

Command Shell::_currentCommand;
