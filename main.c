#include "main.h"

char *extr[156];
int status;

char *builtins[] = {"echo", "printf", "read", 
    "cd","pwd", "pushd", "popd","jobs","fg","bg",
    "dirs", "let", "eval", "set", "unset", "export", "declare",
    "typeset", "readonly", "getopts", "source", "exit", "exec",
    "shopt", "caller", "true", "type", "hash", "bind", "help",
    NULL
};

pid_t pid;
node *head;
char prompt[100];

int main()
{
    system("clear");

    char input[100];

    strcpy(prompt, "MINISHELL$");
    signal(SIGINT,signal_handler);
    signal(SIGTSTP,signal_handler);
    

    extract_external_commands(extr);

   scan_input(prompt,input);
   return 0;
}


void scan_input(char *prompt, char *input_string)
{
    while (1)
    {
        printf(BOLD GREEN"%s "RESET, prompt);

        memset(input_string, '\0', 100);

        if (scanf(" %[^\n]", input_string) != 1)
        {
            continue;
        }

        getchar();   // remove '\n'

        /* PS1 command */
        char *res = strstr(input_string, "PS1");

        if (res != NULL)
        {
            int i = 0;
            int invalid = 0;

            while (input_string[i] != '\0')
            {
                if (input_string[i] == ' ')
                {
                    printf("Invalid cmd!\n");
                    invalid = 1;
                    break;
                }

                i++;
            }

            if (!invalid)
            {
                char *next = res + 4;
                strcpy(prompt, next);
            }

            continue;
        }

        char *command = get_command(input_string);

        if (check_command_type(command) == EXTERNAL)
        {
            pid = fork();

            if (pid > 0)
            {
                int child_status;

                pid_t result;

                do
                {
                    result = waitpid(pid, &child_status, WUNTRACED);
                }
                while (result == -1 && errno == EINTR);

                if (result > 0)
                {
                    if (WIFSTOPPED(child_status))
                    {
                        insertfirst(&head, input_string);

                        printf(RED "\n[%d] Stopped  %s\n" RESET,
                            pid, input_string);
                    }
                    else if (WIFSIGNALED(child_status))
                    {
                        printf("\n");
                    }

                    status = child_status;
                }

                pid = 0;
        }
    
    else if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        execute_external_commands(input_string);

        _exit(127);
    }
    else
    {
        perror("fork");
        pid = 0;
    }
}
        else if (check_command_type(command) == BUILTIN)
        {
            execute_internal_commands(input_string);
        }
        else
        {
            printf(RED"command not found!\n"RESET);
        }

        free(command);
    }
}


char *get_command(char *input_string)
{
    char *command = malloc(20);

    if (command == NULL)
    {
        perror("malloc");
        exit(1);
    }

    int i = 0;

    while (input_string[i] != ' ' &&
           input_string[i] != '\0')
    {
        command[i] = input_string[i];
        i++;
    }

    command[i] = '\0';

    return command;
}


void extract_external_commands(char **extr)
{
    int fd = open("command.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

    char cmd[100];
    char c;

    int j = 0;
    int i = 0;

    while (read(fd, &c, 1) > 0)
    {
        if (c != '\n' && c != '\r')
        {
            cmd[i] = c;
            i++;
        }
        else if (c == '\n')
        {
            cmd[i] = '\0';

            extr[j] = malloc(strlen(cmd) + 1);

            if (extr[j] == NULL)
            {
                perror("malloc");
                close(fd);
                exit(1);
            }

            strcpy(extr[j], cmd);

            j++;
            i = 0;
        }
    }

    if (i > 0)
    {
        cmd[i] = '\0';

        extr[j] = malloc(strlen(cmd) + 1);

        if (extr[j] == NULL)
        {
            perror("malloc");
            close(fd);
            exit(1);
        }

        strcpy(extr[j], cmd);
        j++;
    }

    extr[j] = NULL;

    close(fd);
}

int check_command_type(char *command)
{
    /* External commands */
    for (int i = 0; extr[i] != NULL; i++)
    {
        if (strcmp(extr[i], command) == 0)
        {
            return EXTERNAL;
        }
    }

    /* Built-in commands */
    for (int i = 0; builtins[i] != NULL; i++)
    {
        if (strcmp(builtins[i], command) == 0)
        {
            return BUILTIN;
        }
    }

    return NO_COMMAND;
}


void execute_internal_commands(char *input_string)
{
    char *cmd_with_options[20];

    convert(input_string, cmd_with_options);

    execute(cmd_with_options);
}


int check_pipes(char *cmd[])
{
    int count = 0;

    for (int i = 0; cmd[i] != NULL; i++)
    {
        if (strcmp(cmd[i], "|") == 0)
        {
            count++;
        }
    }

    return count;
}


void convert(char *input, char *cmdo[])
{
    int i = 0;
    int j = 0;

    char cmd[100];
    int k = 0;

    while (input[i] != '\0')
    {
        if (input[i] != ' ')
        {
            cmd[k] = input[i];
            k++;
        }
        else
        {
            if (k > 0)
            {
                cmd[k] = '\0';

                cmdo[j] = malloc(strlen(cmd) + 1);

                if (cmdo[j] == NULL)
                {
                    perror("malloc");
                    exit(1);
                }

                strcpy(cmdo[j], cmd);

                j++;
                k = 0;
            }
        }

        i++;
    }

    /* Store last command */
    if (k > 0)
    {
        cmd[k] = '\0';

        cmdo[j] = malloc(strlen(cmd) + 1);

        if (cmdo[j] == NULL)
        {
            perror("malloc");
            exit(1);
        }

        strcpy(cmdo[j], cmd);

        j++;
    }

    /* VERY IMPORTANT */
    cmdo[j] = NULL;
}


void execute_external_commands(char *input_string)
{
    char *cmd_with_options[20];

    convert(input_string, cmd_with_options);

    if (check_pipes(cmd_with_options) == 0)
    {
        execvp(cmd_with_options[0], cmd_with_options);

        perror("execvp");
    }
    else
    {
        /* Pipe implementation can be added here */
        npipe(cmd_with_options);
    }
}


void execute(char **cmd)
{
    /* exit */
    if (strcmp(cmd[0], "exit") == 0)
    {
        exit(0);
    }


    /* pwd */
    if (strcmp(cmd[0], "pwd") == 0)
    {
        char path[100];

        if (getcwd(path, sizeof(path)) != NULL)
        {
            printf(BLUE"%s\n"RESET, path);
        }

        return;
    }


    /* cd */
    if (strcmp(cmd[0], "cd") == 0)
    {
        if (cmd[1] == NULL)
        {
            printf("cd: missing argument\n");
        }
        else if (chdir(cmd[1]) != 0)
        {
            perror("cd");
        }

        return;
    }


    /* echo */
    if (strcmp(cmd[0], "echo") == 0)
    {
        if (cmd[1] == NULL)
        {
            printf("\n");
            return;
        }

        /* echo $$ */
        if (strcmp(cmd[1], "$$") == 0)
        {
            printf("%d\n", getpid());
            return;
        }

        /* echo $? */
        if (strcmp(cmd[1], "$?") == 0)
        {
            printf("%d\n", WEXITSTATUS(status));
            return;
        }

        /* echo $SHELL */
        if (strcmp(cmd[1], "$SHELL") == 0)
        {
            char *shell = getenv("SHELL");

            if (shell != NULL)
            {
                printf("%s\n", shell);
            }

            return;
        }

        printf("%s\n", cmd[1]);

        return;
    }

        /* jobs */
    if (strcmp(cmd[0], "jobs") == 0)
    {
        execute_jobs(cmd);
        return;
    }

    /* bg */
    if (strcmp(cmd[0], "bg") == 0)
    {
        execute_bg(cmd);
        return;
    }

    /* fg */
    if (strcmp(cmd[0], "fg") == 0)
    {
        execute_fg(cmd);
        return;
    }
}
void npipe(char *argv[])
{
    int i = 0;
    int pipecount = 0;
    int arr[20];
    int j = 1;

    arr[0] = 0;

    /* Find all pipes */
    while (argv[i] != NULL)
    {
        if (strcmp(argv[i], "|") == 0)
        {
            if (i == 0 || argv[i + 1] == NULL ||
                strcmp(argv[i + 1], "|") == 0)
            {
                printf("Error: Invalid pipe\n");
                return;
            }

            argv[i] = NULL;
            arr[j++] = i + 1;
            pipecount++;
        }

        i++;
    }

    if (pipecount == 0)
    {
        printf("Error: No pipe found\n");
        return;
    }

    int prev_fd = -1;

    for (i = 0; i <= pipecount; i++)
    {
        int fd[2];

        if (i < pipecount)
        {
            if (pipe(fd) == -1)
            {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");
            return;
        }

        if (pid == 0)
        {
            /* Take input from previous pipe */
            if (prev_fd != -1)
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            /* Send output to next pipe */
            if (i < pipecount)
            {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            execvp(argv[arr[i]], &argv[arr[i]]);

            perror("execvp");
            exit(1);
        }
        else
        {
            /* Parent */

            if (prev_fd != -1)
            {
                close(prev_fd);
            }

            if (i < pipecount)
            {
                close(fd[1]);
                prev_fd = fd[0];
            }
        }
    }

    /* Wait for all children */
    for (i = 0; i <= pipecount; i++)
    {
        wait(NULL);
    }
}
void signal_handler(int sig_num)
{
    if(sig_num==SIGINT)
    {
        if(pid==0)
        {
            printf(BOLD GREEN"\n%s "RESET, prompt);
            fflush(stdout);
        }
    }
    if(sig_num==SIGTSTP)
    {
        if(pid==0)
        {
            printf(BOLD GREEN"\n%s "RESET, prompt);
            fflush(stdout);
        }
    }
}
void insertfirst(node **head, char *string)
{
    node *newnode = malloc(sizeof(node));

    if (newnode == NULL)
    {
        perror("malloc");
        return;
    }

    snprintf(newnode->process, sizeof(newnode->process),
             "%s", string);

    newnode->child_pid = pid;
    newnode->state = STOPPED;

    newnode->link = *head;
    *head = newnode;
}
void deletefirst(node **head, pid_t child_pid)
{
    node *temp = *head;
    node *prev = NULL;

    while (temp != NULL)
    {
        if (temp->child_pid == child_pid)
        {
            if (prev == NULL)
                *head = temp->link;
            else
                prev->link = temp->link;

            free(temp);
            return;
        }

        prev = temp;
        temp = temp->link;
    }
}
void printlist(node **head)
{
    node *temp = *head;
    node *next;

    while (temp != NULL)
    {
        next = temp->link;

        if (temp->state == STOPPED)
        {
            printf(RED "[%d] Stopped  %s\n" RESET,
                   temp->child_pid, temp->process);
        }
        else if (temp->state == RUNNING)
        {
            printf(YELLOW "[%d] Running  %s\n" RESET,
                   temp->child_pid, temp->process);
        }
        else if (temp->state == DONE)
        {
            printf(GREEN "[%d] Done     %s\n" RESET,
                   temp->child_pid, temp->process);

            deletefirst(head, temp->child_pid);
        }

        temp = next;
    }
}
void update_job_states(void)
{
    node *temp = head;
    int job_status;

    while (temp != NULL)
    {
        pid_t result = waitpid(
            temp->child_pid,
            &job_status,
            WNOHANG | WUNTRACED | WCONTINUED
        );

        if (result > 0)
        {
            if (WIFEXITED(job_status) ||
                WIFSIGNALED(job_status))
            {
                temp->state = DONE;
            }
            else if (WIFSTOPPED(job_status))
            {
                temp->state = STOPPED;
            }
            else if (WIFCONTINUED(job_status))
            {
                temp->state = RUNNING;
            }
        }

        temp = temp->link;
    }
}
void execute_jobs(char **cmd)
{
    (void)cmd;

    update_job_states();
    printlist(&head);
}
void execute_bg(char **cmd)
{
    (void)cmd;

    if (head == NULL)
    {
        printf("bg: no current job\n");
        return;
    }

    node *job = head;

    if (job->state != STOPPED)
    {
        printf("bg: job is not stopped\n");
        return;
    }

    if (kill(job->child_pid, SIGCONT) == -1)
    {
        perror("bg");
        return;
    }

    job->state = RUNNING;

    printf(YELLOW "[%d] Running  %s &\n" RESET,
           job->child_pid, job->process);
}
void execute_fg(char **cmd)
{
    (void)cmd;

    if (head == NULL)
    {
        printf("fg: no current job\n");
        return;
    }

    node *job = head;
    pid_t job_pid = job->child_pid;

    if (kill(job_pid, SIGCONT) == -1)
    {
        perror("fg");
        return;
    }

    job->state = RUNNING;
    pid = job_pid;

    printf("%s\n", job->process);

    int job_status;
    pid_t result;

    do
    {
        result = waitpid(job_pid, &job_status, WUNTRACED);
    }
    while (result == -1 && errno == EINTR);

    if (result == -1)
    {
        perror("waitpid");
        pid = 0;
        return;
    }

    if (WIFSTOPPED(job_status))
    {
        job->state = STOPPED;

        printf(RED "\n[%d] Stopped  %s\n" RESET,
               job_pid, job->process);
    }
    else if (WIFEXITED(job_status) ||
             WIFSIGNALED(job_status))
    {
        deletefirst(&head, job_pid);
    }

    pid = 0;
    status = job_status;
}