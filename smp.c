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



typedef void APIPGM();
typedef void CFUNC();
#pragma linkage(APIPGM,OS)

typedef _Packed struct QUERY_PARMS
{
  char *csi;
  long csilen;
  char *zone;
  long zonelen;
  char *entrytype;
  long entrylen;
  char *subentrytype;
  long subentrylen;
  char *filter;
  long filterlen;
} QUERY_PARMS, * P_QUERY_PARMS;

typedef _Packed struct API_Version
{
  char apiver[2];
  char apirel[2];
  char apimod[2];
  char apiptf[2];
} API_VERSION, * P_API_VERSION;

typedef _Packed struct CSI_ENTRY
{
  _Packed struct CSI_ENTRY *next;
  _Packed struct SUBENTRY *subentries;
  char entryname[8];
  char zonename[7];
} CSI_ENTRY, * P_CSI_ENTRY;

typedef _Packed struct ENTRY_LIST
{
  _Packed struct ENTRY_LIST *next;
  _Packed struct CSI_ENTRY *entries;
  char type[12];
} ENTRY_LIST, * P_ENTRY_LIST;

typedef _Packed struct ITEM_LIST
{
  _Packed struct ITEM_LIST *next;
  long datalen;
  char *data;
} ITEM_LIST, * P_ITEM_LIST;


typedef _Packed struct SUBENTRY
{
  _Packed struct SUBENTRY *next;
  void *subentrydata;
  char type[12];
} SUBENTRY, * P_SUBENTRY;

typedef _Packed struct VER
{
  _Packed struct VER *next;
  _Packed struct SUBENTRY *verdata;
  char vernum[3];
} VER, * P_VER;

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

  APIPGM * gimapi;
  gimapi = (APIPGM *) fetch("GIMAPI");
  if (verbose){
    fprintf(stderr, "GIMAPI at 0x%08x\n", (uint32_t) gimapi);
  }

  ITEM_LIST* msgbuff = 0;

  P_API_VERSION verout;

  int rc = 0, cc = 0;
  
  (*gimapi) ("VERSION ", 0, &verout, "ENU", &rc, &cc, &msgbuff);

  if (verbose){
    const char* version_string = (const char*)verout;
    fprintf(stderr,
            "API version string: %s, rc = %d, cc = %d\n",
            (rc == 0 ? version_string : "UNKNOWN"),
            rc, cc);
  }
  (*gimapi) ("FREE    ", 0, 0, "ENU", &rc, &cc, &msgbuff);
  if (verbose){
    fprintf(stderr,
            "FREE returned rc = %d, cc = %d\n",
            rc, cc);
  }

  /* (*gimapi) (apicmd,&parmptr,&outptr,language,&rc,&cc,&msgbuff); */
  release((CFUNC*) gimapi);
  return 0;
}
