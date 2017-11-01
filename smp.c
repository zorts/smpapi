#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <_Nascii.h> /* for __isASCII() */


static void usage(int argc, char** argv){
  fprintf(stderr, 
          "usage: %s\n"
          ,
          argv[0]);
}

int main(int argc, char**argv){
  opterr = 0; /* disable auto error reporting */
  char opt = 0;
  /* These copies are needed because optind and optarg aren't
     necessarily visible to debuggers, and you often want them. */
  int myoptind = 1;
  char* myoptarg = 0;
  bool verbose = false;

  while (((char) -1) != (opt = (char) getopt(argc, argv, "v"))){
    myoptind = optind;
    myoptarg = optarg;

    switch(opt){

    case 'v':
      verbose = true;
      break;

    default:
      {
        char erropt = optopt;
        fprintf(stderr, "unrecognized option '%c'\n", erropt);
      }
      break;
    }
  }

  if (myoptind < argc){
    fprintf(stderr, "unused arguments:");
    while (myoptind < argc){
      fprintf(stderr, " %s", argv[myoptind++]);
    }
    fprintf(stderr, "\n");
  }

 return 0;
}
