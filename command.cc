/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>

#include <sys/stat.h>

#include "command.hh"
#include "shell.hh"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "common.hh"

//pids = std::vector<int>();

//std::string argu;

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
    _append = false;
    _valid = true;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}
void Command::it() {
    std::vector<std::string> args;
    for (auto simpleCommand : _simpleCommands) {
        for (auto arg: simpleCommand->_arguments) {
            args.push_back(*arg);
        }
    }
    //print(args);
    for (size_t i = 0; i < args.size(); i++) {
        
        if (i == args.size() - 1) {
            setenv("_", args[i].c_str(), 1);
            
        }
    }
    
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    bool eq = false;
    _simpleCommands.clear();

    if (_outFile != NULL && _errFile != NULL && *_outFile == *_errFile) {
        eq = true;

    }

    if ( _outFile ) {
        delete _outFile;
    }
    _outFile = NULL;
    if (eq) {
        //delete _errFile;
        _errFile = NULL;
    }

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::prints() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );

    
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }
    if (!_valid) {
        //if the command has multiple redirections
        Shell::prompt();

        return;
    }
    
    

    // Print contents of Command data structure
    //print();
    //it();
    
    
    int tempin = dup(0);
    int tempout = dup(1);
    int temperr = dup(2);
    //int temperr = dup(2);
    int fdin = 0;
    int ret = 0;
    

    

    if (_inFile) {
        //if the input file is not Null
        fdin = open(_inFile->c_str(), O_RDONLY);
    } else {
        fdin = dup(tempin);
    }
    int fdout = 0;
    int fderr = 0;
    if (_errFile) {
        //derr = open(_errFile->c_str(), O_WRONLY | O_CREAT, 0600);
        //std::cout << *_errFile;
        if (_append) {
            fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND, 0600);

        } else {
            fderr = open(_errFile->c_str(), O_WRONLY | O_CREAT, 0600);
        }
    } else {
        fderr = dup(temperr);
    }

    if (strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "exit") == 0) {
       //std::cout<<"Good Bye !!\n";
        close(tempin);
        close(tempout);
        close(temperr);
        close(fdin);
        close(fderr);
        exit(1);
    }
    dup2(fderr, 2);
    close(fderr);
    

    
    

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    bool fo = true;
    for (size_t i = 0; i < _simpleCommands.size(); i++) {
        dup2(fdin, 0);
        close(fdin);
        SimpleCommand *command = _simpleCommands[i];
        if (strcmp(command->_arguments[0]->c_str(), "setenv") == 0) {
            fo = false;
            if (setenv(command->_arguments[1]->c_str(), command->_arguments[2]->c_str(), 1) < 0) {
                perror("setenv");
            }

            


        }
        if (strcmp(command->_arguments[0]->c_str(), "unsetenv") == 0) {
            fo= false;
            if (unsetenv(_simpleCommands[i]->_arguments[1]->c_str()) < 0) {
                perror("unsetenv");
            }
            


        }
        //if (strcmp(command->_arguments[0]->c_str(), "source")) {


            


        //}
        if (strcmp(command->_arguments[0]->c_str(), "cd") == 0) {
            fo = false;
            //cout << "Home";
            if (command->_arguments.size() == 1) {
                //go to home
                if (chdir(getenv("HOME")) < 0) {
                    perror("error");
                }
                
            } else {
                if (chdir(command->_arguments[1]->c_str()) < 0) {
                    std::string message = "cd: can't cd to ";
                    message = message + *(command->_arguments[1]);
                    perror(message.c_str());

                }
            }
            


        }
        
        if (i == _simpleCommands.size() - 1) {
            //last simple command
            if (_outFile) {
                if (_append) {
                    fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND, 0600);

               } else {
                    fdout = open(_outFile->c_str(), O_WRONLY | O_CREAT, 0600);
               }
                
            } else {
                fdout = dup(tempout);
            }
    
            
        } else {
            //create pipe
            int fdpipe[2];
            pipe(fdpipe);
            fdin = fdpipe[0];
            fdout = fdpipe[1];

        }
        dup2(fdout, 1);
        close(fdout);
        if (fo) {
            ret = fork();
            //har ** environ;
            if (ret == 0) {
                //in the child process
                if (strcmp(command->_arguments[0]->c_str(), "printenv") == 0) {
                    char **p = environ;
                    while (*p != NULL) {
                        printf("%s\n",*p);
                        p++;
                    }
                    close(tempin);
                    close(tempout);
                    close(temperr);
                    //close()
                    exit(0);

                }
                char ** arr = (char**)malloc((_simpleCommands[i]->_arguments.size() + 1) * sizeof(char *));
                for (size_t j = 0; j < command->_arguments.size(); j++) {
                    arr[j] = strdup(command->_arguments[j]->c_str());
                }
                
                
                arr[_simpleCommands[i]->_arguments.size()] = NULL;
                //std::cout << *(command->_arguments[_simpleCommands[i]->_arguments.size() - 1]) + '\n';
                //bool argu = false;
                //if ((*command->_arguments[0]).compare("echo")) {
                    //argu = true;
                    //std::cout << "not equal\n";

                //setenv("_", arr[_simpleCommands[i]->_arguments.size() - 1], 1);
                //argu = *(command->_arguments[_simpleCommands[i]->_arguments.size() - 1]);
               // }
                
                //std::string e = getenv("_");
                //std::cout << e + '\n';
                execvp(command->_arguments[0]->c_str(),arr);
                perror("execvp");
                close(tempin);
                close(tempout);
                close(temperr);
                _exit(1);


            } else if (ret < 0) {
                
                perror("fork");
                return;

            } 
        }
    }
    dup2(tempin, 0);
    dup2(tempout, 1);
    dup2(temperr, 2);
    close(tempin);
    close(tempout);
    close(temperr);
    std::string p = "";

    p = std::to_string(getpid());
        //std::cout << p + '\n';
    setenv("!",p.c_str(), 1);
    

    if (!_background) {
        int o;
        waitpid(ret, &o, 0);
        if ( WIFEXITED(o) ) {
            int es = WEXITSTATUS(o);
            std::string s = std::to_string(es);
            setenv("?",s.c_str(), 1);

            
            //printf("Exit status was %d\n", es);
        }
        
    
        //pids.push_back(get_pid());
    }
    
    
    
    
    

    

    // Clear to prepare for next command
    it();
    clear();

    // Print new prompt
   
    Shell::prompt(); 
    
    
}


SimpleCommand * Command::_currentSimpleCommand;
