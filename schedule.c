
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#include "schedule.h"


int pIndex = -1;
process_t pTable[MAX_PROCESSES] = {{NULL, {NULL}, 0}};


void timer_handler(int signum) {
   setTimer(OFF);
   if (kill(pTable[pIndex].pid, SIGSTOP) == -1) {
      if (errno != ESRCH) {
         perror("kill failed in timer");
         exit(-1);
      }
   }
}

int main(int argc, char *argv[]) {

   int quantum = -1;
   int numProcs = -1;
   int status = -1;

   if (argc < 3) {
      usage();
   }

   setupSignalHandling();

   quantum = atoi(argv[1]);
   if (quantum <= 0) {
      printf("error: quantum must be greater than 0\n");
      exit(-1);
   }

   numProcs = processArgs(argc, argv);

   while (numProcs > 0) {

      // use round robin to get next index
      pIndex = roundRobin(pIndex, numProcs);

      // resume process/child
      resumeProcess(pIndex);
      setTimer(quantum);

      status = -1;
      // wait() for either itimer or child to end
      if (waitpid(pTable[pIndex].pid, &status, WUNTRACED) == -1) {
         if (errno != EINTR) {
            perror("waitpid failed");
            exit(-1);
         }
      }

      setTimer(OFF);

      // if child ended
      if (WIFEXITED(status)) {
         // adjust ptable
         adjustPtable(pIndex);
         numProcs--;
         pIndex--;
      }
   }

   return 0;
}

void usage(void) {
   printf("schedule quantum [prog 1 [args] [: prog 2 [args] [: ... ]]]\n");
   exit(-1);
}

int processArgs(int argc, char *argv[]) {
   int curProcess = 0;
   int curArg = 0;

   int i;
   for (i = 2; i < argc; i++) {
      if (strcmp(argv[i], ":") == 0) { // next process
         curProcess++;
         curArg = 0;
         if (curProcess == MAX_PROCESSES) {
            return curProcess;
         }
         continue;
      }

      if (curArg == 0) { // process name
         pTable[curProcess].name = argv[i];
      }

      // list of arguments
      if (curArg != (MAX_ARGUMENTS + 1)) {
         pTable[curProcess].args[curArg++] = argv[i];
      }
   }

   return curProcess + 1; 
}

void setupSignalHandling(void) {
   struct sigaction sa;

   memset(&sa, 0, sizeof(sa));
   sa.sa_handler = &timer_handler;

   if (sigaction(SIGALRM, &sa, NULL) == -1) {
      perror("sigaction failed");
      exit(-1);
   }

   return;
}

void setTimer(int quantum) {
   struct itimerval timer;

   timer.it_value.tv_sec = quantum / 1000;
   timer.it_value.tv_usec = (quantum % 1000) * 1000;  // millisec

   timer.it_interval.tv_sec = quantum / 1000;
   timer.it_interval.tv_usec = (quantum % 1000) * 1000;  // millisec

   if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
      perror("setitimer failed");
      exit(-1);
   }

   return;
}

void adjustPtable(int index) {
   int i = 0;
   for (i = index + 1; i < MAX_PROCESSES; i++) {
      memcpy((void *)&(pTable[i - 1]), (void *)&(pTable[i]), sizeof(process_t));
   }

   return;
}

int roundRobin(int pIndex, int numProcs) {
   if (numProcs <= 0) {
      return -1;
   }

   return ((pIndex + 1) % numProcs);
}

void resumeProcess(int pIndex) {
   if (pTable[pIndex].pid == 0) { // starting for the first time
      if ((pTable[pIndex].pid = fork()) == -1) {
         perror("fork failed");
         exit(-1);
      }
      if (pTable[pIndex].pid == 0) { // child
         if (execv(pTable[pIndex].name, pTable[pIndex].args) == -1) {
            perror("execv failed");
            exit(-1);
         }
      }
   } else { // previously started, needs to resumed
      if (kill(pTable[pIndex].pid, SIGCONT) == -1) {
         perror("kill failed");
         exit(-1);
      }
   }

   return;
}
