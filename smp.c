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

static const char defaultCSI[] = "SMPE.ZOSV201.GLOBAL.CSI";
static const char defaultZone[] = "*";
static const char defaultEntry[] = "*";
static const char defaultSubEntry[] = "*";
static const char defaultFilter[] = "";

int main(int argc, char**argv){
  opterr = 0; /* disable auto error reporting */
  char opt = 0;
  /* These copies are needed because optind and optarg aren't
     necessarily visible to debuggers, and you often want them. */
  int myoptind = 1;
  char* myoptarg = 0;
  bool verbose = false;
  bool messages = false;
  char* csi = (char*) defaultCSI;
  char* zone = (char*) defaultZone;
  char* entry = (char*) defaultEntry;
  char* subentry = (char*) defaultSubEntry;
  char* filter = (char*) defaultFilter;

  while (((char) -1) != (opt = (char) getopt(argc, argv, "c:z:e:s:f:vm"))){
    myoptind = optind;
    myoptarg = optarg;

    switch(opt){

    case 'c':
      csi = myoptarg;
      break;

    case 'z':
      zone = myoptarg;
      break;

    case 'e':
      entry = myoptarg;
      break;

    case 's':
      subentry = myoptarg;
      break;

    case 'f':
      filter = myoptarg;
      break;

    case 'v':
      verbose = true;
      messages = true;
      break;

    case 'm':
      messages = true;
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
  int rc = 0, cc = 0;
  
  if (verbose){
    P_API_VERSION verout;
    (*gimapi) ("VERSION ", 0, &verout, "ENU", &rc, &cc, &msgbuff);
    const char* version_string = (const char*)verout;
    fprintf(stderr,
            "API version string: %s, rc = %d, cc = %d\n",
            (rc == 0 ? version_string : "UNKNOWN"),
            rc, cc);
    (*gimapi) ("FREE    ", 0, 0, "ENU", &rc, &cc, &msgbuff);
      fprintf(stderr,
            "FREE returned rc = %d, cc = %d\n",
            rc, cc);
  }

  QUERY_PARMS parms;
  P_QUERY_PARMS parmptr = &parms;
  void* outptr = 0;
  parms.csi = csi;
  parms.csilen = (long) strlen(csi);
  parms.zone = zone;
  parms.zonelen = (long) strlen(zone);
  parms.entrytype = entry;
  parms.entrylen = (long) strlen(entry);
  parms.subentrytype = subentry;
  parms.subentrylen = (long) strlen(subentry);
  parms.filter = filter;
  parms.filterlen = (long) strlen(filter);
  msgbuff = 0;

  (*gimapi) ("QUERY   ", &parmptr, &outptr, "ENU", &rc, &cc, &msgbuff);
  if (verbose){
    fprintf(stderr,
            "QUERY returned rc = %d, cc = %d\n",
            rc, cc);
  }
  if ((msgbuff != 0) && ((rc > 4) || messages)){
    if (!verbose){ /* don't do it twice */
      fprintf(stderr,
              "QUERY returned rc = %d, cc = %d\n",
              rc, cc);
    }

    char msgcopy[1000];
    while (msgbuff){
      size_t copylen = msgbuff->datalen > (sizeof(msgcopy)-1) 
                       ? (sizeof(msgcopy)-1)
                       : (size_t) (msgbuff->datalen);
      memcpy(msgcopy, msgbuff->data, copylen);
      msgcopy[copylen] = 0;
      fprintf(stderr, "%s\n", msgcopy);
      msgbuff = msgbuff->next;
    }
  }

  (*gimapi) ("FREE    ", 0, 0, "ENU", &rc, &cc, &msgbuff);

  release((CFUNC*) gimapi);
  return 0;
}
