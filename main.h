#ifndef main_h
#define main_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include<errno.h>
 

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3

#define STOPPED 4
#define RUNNING 5
#define DONE    6

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define BOLD "\033[1m"
#define RESET   "\x1b[0m"

typedef struct NODE
{
    char process[100];
    pid_t child_pid;
    int state;
    struct NODE *link;
} node;

void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);

void copy_change(char *prompt, char *input_string);

int check_command_type(char *command);
void echo(char *input_string, int status);
void execute_internal_commands(char *input_string);
void execute_external_commands(char *input_string);
void signal_handler(int sig_num);
void extract_external_commands(char **external_commands);
int check_pipes(char*cmd[]);
void convert(char*input,char*cmdo[]);
void execute(char**cmd);
void npipe(char *argv[]);


void insertfirst(node **head, char *string);
void deletefirst(node **head, pid_t child_pid);
void printlist(node **head);

void execute_jobs(char **cmd);
void execute_fg(char **cmd);
void execute_bg(char **cmd);
void update_job_states(void);


#endif