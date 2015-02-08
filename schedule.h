
#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include <sys/types.h>

/* #define's */
#define MAX_PROCESSES 10
#define MAX_ARGUMENTS 10

#define OFF 0

/* structures */
typedef struct {
   char *name;
   char *args[MAX_ARGUMENTS + 1];
   pid_t pid;
} process_t;

/* function prototypes */
void usage(void);
void setupSignalHandling(void);
int processArgs(int argc, char *argv[]);
void setTimer(int quantum);
void adjustPtable(int index);
int roundRobin(int pIndex, int numProc);
void resumeProcess(int pIndex);

#endif //__SCHEDULER_H
