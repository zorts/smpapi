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
#include "smpapi.h"

static const char defaultCSI[] = "SMPE.ZOSV201.GLOBAL.CSI";
static const char defaultZone[] = "*";
static const char defaultEntry[] = "*";
static const char defaultSubEntry[] = "*";
static const char defaultFilter[] = "";

static 
void printString(FILE* file, const char* item, long itemLength){
  char msgcopy[1000];
  size_t copylen = itemLength > (sizeof(msgcopy)-1) 
    ? (sizeof(msgcopy)-1)
    : (size_t) (itemLength);
  memcpy(msgcopy, item, copylen);
  msgcopy[copylen] = 0;
  fprintf(stderr, "%s\n", msgcopy);
}

static
void  printItem(FILE* file, const P_ITEM_LIST item){
  printString(file, item->data, item->datalen);
}

static
void usage(int argc, char**argv){
  fprintf(stderr,
          "usage: %s \n"
          "  -h    get usage help \n"
          "  -v    verbose \n"
          "  -m    display messages, even if return code is 0 or 4 \n"
          "  -c <CSI>  DSN of CSI to use, fully qualified, no quoting required, lower case OK \n"
          "  -z <zone(s)>  Zone selection: \n"
          "     global - Use the global zone \n"
          "     alltzones - Use all target zones \n"
          "     alldzones - Use all DLIB zones \n"
          "     * (default) - Use all zones \n"
          "     zone[,zone] - Use specific names zone(s) \n"
          "  -e <entry type(s)>  Type(s) of entries to be displayed, such as: \n"
          "     assem, dddef, dlib, element, lmod, mac, sysmod, etc. \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/entry.htm \n"
          "  -s <subtype(s)>  Subtypes() of entries to be displayed \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/subent.htm \n"
          "  -f <filter>  Filter expression \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/filter.htm \n"
          " \n"
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
  bool messages = false;
  char* csi = (char*) defaultCSI;
  char* zone = (char*) defaultZone;
  char* entry = (char*) defaultEntry;
  char* subentry = (char*) defaultSubEntry;
  char* filter = (char*) defaultFilter;

  while (((char) -1) != (opt = (char) getopt(argc, argv, "c:z:e:s:f:vmh"))){
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

    case 'h':
      usage(argc, argv);
      return 1;
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
      printItem(stderr, msgbuff);
      msgbuff = msgbuff->next;
    }
  }


  (*gimapi) ("FREE    ", 0, 0, "ENU", &rc, &cc, &msgbuff);

  release((CFUNC*) gimapi);
  return 0;
}
