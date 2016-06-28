/* test-runcommand-v1 - Program to test runcommand function version 1. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <foo.h>
#include <debug.h>

#define LOGFILE "foo.log"
#define EXECOK 0
#define EXECFAIL 1

int go_on = 1;
void (*runcommand_onexit)(void);

void finish (void)
{
  go_on = 0;
}

/* Run the = finish"program with arguments and report results. */

int tryrun (char **args,  int correct_exit_status, int correct_termination, 
	    int *io, int execresult, int mode)
{
  int pid, nbytes, test, count;
  command_t command;


  /* Run the command.*/

  command.args = args;
  command.io = io;
  command.mode = mode;
  runcommand_onexit = finish;

  pid = runcommand (&command);
  if (pid==-2)
    {
      printf ("%s %s (pid %d) execution failed %n", 
	      args[0], args[1], pid, &nbytes);
      test = execresult == EXECFAIL ? 1 : 0;
      printf ("%*s\n", 80-nbytes, test ? "OK" : "NO");
      return test;
    }
  
  if (command.mode == NONBLOCK)
    {
      printf ("%s %s (pid %d) execution started asynchronously %n", 
	      args[0], args[1], pid, &nbytes);
      count =0;
      while (go_on)
	{
	  sleep(1);
	  count++;
	}
      test = (count == 2) ? 1 : 0;
      printf ("%*s\n", 80-nbytes, test ? "OK" : "NO");
      return test;
    }
  

  printf ("%s %s (pid %d) terminated %s and %s (status %d) %n", 
	  args[0], args[1], pid, 
	  command.termination == NORMAL_TERMINATION ? "normally" : "abnormally", 
	  command.exit_status == EXIT_SUCCESS ? "sucessfully" : "unsucessfully", 
	  command.exit_status, &nbytes);

  test =  (command.exit_status == correct_exit_status) 
    && (command.termination == correct_termination);



  printf ("%*s\n", 80-nbytes, test ? "OK" : "NO");

  return test;
}


/* Main function: returns successfully if all tests are ok;
   returns unsucessfully otherwise. */

int main (int argc, char **argv)
{
  int result, fd;
  /* int count; */

  /* Test cases. */

  char *args1[] = {"ls", "runcommand.c", NULL}; /* File does exist. */
  char *args2[] = {"ls", "nosuchfile", NULL};   /* File does not exist. */
  char *args3[] = {"./segfault", "", NULL};     /* Abnormal termination. */
  char *args4[] = {"./io", NULL};		/* Test IO redirection. */
  char *args5[] = {"./nosuchfile", NULL};	/* Exec failed. */
  char *args6[] = {"./delay", NULL};	        /* Test nonblock. */

  int io[3], io2[3];

  result = EXIT_SUCCESS;

  /* Disable standard streams for convenience. */
  fatal ((io[0] = open ("/dev/null", O_WRONLY)) <0);
  fatal ((io[1] = open ("/dev/null", O_WRONLY)) <0);
  fatal ((io[2] = open ("/dev/null", O_WRONLY)) <0);


  result |=tryrun (args1, 0, NORMAL_TERMINATION, io, EXECOK, BLOCK);   /* Normal, success. */
  result |=tryrun (args2, 2, NORMAL_TERMINATION, io, EXECOK, BLOCK);   /* Normal, failire. */
  result |=tryrun (args3, EXIT_FAILURE, ABNORMAL_TERMINATION, io, EXECOK, BLOCK); /* Abnormal (success). */


  /* Test redirection. */

  fatal ((fd = open ("in.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0);
  write (fd, "a", 1);
  fatal(close(fd)<0);

  fatal ((io2[0] = open ("in.txt", O_RDONLY)) <0);
  fatal ((io2[1] = open ("out.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0);
  fatal ((io2[2] = open ("err.txt", O_CREAT | O_TRUNC | O_RDWR,  S_IRUSR | S_IWUSR)) <0);


  result |=tryrun (args4, 0, NORMAL_TERMINATION, io2, EXECOK, BLOCK);   /* Normal, success. */

  /* Test whether exec failed. */

  result |=tryrun (args5, 0, 0, io, EXECFAIL, BLOCK);   /* Normal, success. */

  /* Test nonblock. */

  result |=tryrun (args6, 0, 0, io2, EXECFAIL, NONBLOCK);   /* Normal, success. */

  /* count =0; */
  /* while (go_on) */
  /*   { */
  /*     sleep(1); */
  /*     count++; */
  /*   } */

  /* printf ("ok, count %d\n", count); */

  return result;
}
