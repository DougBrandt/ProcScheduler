
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


void usage(void) {
   printf("error: two [indent number]\n");
   exit(-1);
}

int main(int argc, char *argv[]) {

   if (argc < 2) {
      usage();
   }

   int indent = atoi(argv[1]);
   int i;
   printf("-> %d\n", indent);
   fflush(stdout);
   for (i = 0; i < indent; i++) {
      int j;
      raise(SIGSTOP);
      for (j = 0; j < indent; j++) {
         printf("\t");
      }
      printf("%d\n", indent);
      fflush(stdout);
   }

   return 0;
}
