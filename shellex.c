/* $begin shellmain */
#include "csapp.h"
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
sigjmp_buf buf;

void handler(int sig){
  siglongjmp(buf, 1);
}

int main(int argc, char *argv[]) 
{
  char cmdline[MAXLINE]; /* Command line */

  if (!sigsetjmp(buf, 1)){
    Signal(SIGINT, handler);
  }else {
    printf("\n");
  }

  while (1) {
    /* Read */
    if (argc > 1){
      if (!strcmp(argv[1], "-p"))
        printf("%s>", argv[2]);
    } else {
      printf("sh257>");
    }
    Fgets(cmdline, MAXLINE, stdin); 
    if (feof(stdin))
      exit(0);

    /* Evaluate */
    eval(cmdline);
  } 
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
  char *argv[MAXARGS]; /* Argument list execve() */
  char buf[MAXLINE];   /* Holds modified command line */
  int bg;              /* Should the job run in bg or fg? */
  pid_t pid;           /* Process id */
  int child_status;
  int status;

  strcpy(buf, cmdline);
  bg = parseline(buf, argv); 
  if (argv[0] == NULL)  
    return;   /* Ignore empty lines */

  if (!builtin_command(argv)) { 
    if ((pid = Fork()) == 0) {   /* Child runs user job */
      if (execvp(argv[0], argv) < 0) {
        printf("Execution failed (in fork)\n");
        printf("%s: Command not found.\n", argv[0]);
        exit(1);
      }
    }

    /* Parent waits for foreground job to terminate */
    if (!bg) {
      if (waitpid(pid, &status, 0) < 0)
        unix_error("waitfg: waitpid error");
    }
    else
      printf("%d %s\n", pid, cmdline);
    wait(&status);
    if (WIFEXITED(status)){
      printf("Process exited with status code %d\n", WEXITSTATUS(status));
    }
  }

  return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
  if (!strcmp(argv[0], "exit")) /* quit command */
    raise(SIGTERM); //raise() call has to be implemented here;  
  if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
    return 1;
  if (!strcmp(argv[0], "pid"))
    return printf("pid: %d\n", getpid());
  if (!strcmp(argv[0], "ppid"))
    return printf("ppid: %d\n", getppid());
  if (!strcmp(argv[0], "cd")){
    if (argv[1] == NULL){
      return 1;
    }
    chdir(argv[1]);
    return 1;
  }
 if (!strcmp(argv[0], "help")){
  printf("***********************************************************************\n");
  printf("A Custom Shell for CMSC 257\n");
  printf("  - Kyle Jones\n");
  printf("Usage:\n");
  printf("  - To change the shell prompt, you have to restart the shell, and using the -p option along with your desired prompt name after. Example: ./sh257 -p <promptName>\n\n");
  printf("  - A list of built in commands include:\nexit\npid\nppid\ncd\nhelp\n\n");
  printf("  - For more support on non-built-in commands, please refer to the manual. To access the manual, type: man <non-built-in command>\n");
  printf("*****************************************************************************\n");
  return 1;
 }

  return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
  char *delim;         /* Points to first space delimiter */
  int argc;            /* Number of args */
  int bg;              /* Background job? */

  buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
  while (*buf && (*buf == ' ')) /* Ignore leading spaces */
    buf++;

  /* Build the argv list */
  argc = 0;
  while ((delim = strchr(buf, ' '))) {
    argv[argc++] = buf;
    *delim = '\0';
    buf = delim + 1;
    while (*buf && (*buf == ' ')) /* Ignore spaces */
      buf++;
  }
  argv[argc] = NULL;

  if (argc == 0)  /* Ignore blank line */
    return 1;

  /* Should the job run in the background? */
  if ((bg = (*argv[argc-1] == '&')) != 0)
    argv[--argc] = NULL;

  return bg;
}
/* $end parseline */


