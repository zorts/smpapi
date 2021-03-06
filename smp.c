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

static const char defaultCSI[] = "SMPE.ZOSV203.GLOBAL.CSI";
static const char defaultZone[] = "*";
static const char defaultEntry[] = "*";
static const char defaultSubEntry[] = "*";
static const char defaultFilter[] = "";

typedef struct{
  const char* abbrev;
  const char* full;
} csi_abbrev_t;

static csi_abbrev_t abbreviations[] ={
  {"2.1", "SMPE.ZOSV201.GLOBAL.CSI"},
  {"2.2", "SMPE.ZOSV202.GLOBAL.CSI"},
  {"2.3", "SMPE.ZOSV203.GLOBAL.CSI"},
  {0,0}
};

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
const char* currentCSI(const char* candidate){
  if (!candidate){ return defaultCSI;}
  const csi_abbrev_t* pair = abbreviations;
  while (pair->abbrev){
    if (0 == strcmp(pair->abbrev, candidate)){
      return pair->full;
    }
    ++pair;
  }
  return candidate;
}

static
void usage(int argc, char**argv){
  fprintf(stderr,
          "usage: %s \n"
          "  -h    get usage help \n"
          "  -v    verbose \n"
          "  -d    debug \n"
          "  -m    display messages, even if return code is 0 or 4 \n"
          "  -H    produce a header line in the output \n"
          "  -c <CSI>  DSN of CSI to use, fully qualified, no quoting required, lower case OK \n"
          "     default: \"%s\" \n"
          "     You can also use an abbreviation of 2.1, 2.2 or 2.3 to get\n"
          "     the appropriate Rocket default CSI for that release.\n"
          "  -z <zone(s)>  Zone selection: \n"
          "     global - Use the global zone \n"
          "     alltzones - Use all target zones \n"
          "     alldzones - Use all DLIB zones \n"
          "     * - Use all zones \n"
          "     zone[,zone] - Use specific names zone(s) \n"
          "     default: \"%s\" \n"
          "  -e <entry type(s)>  Type(s) of entries to be displayed, such as: \n"
          "     assem, dddef, dlib, element, lmod, mac, sysmod, etc. \n"
          "     * - all entry types \n"
          "     default: \"%s\" \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/entry.htm \n"
          "  -s <subtype(s)>  Subtypes() of entries to be displayed \n"
          "     * - all subtypes types \n"
          "     default: \"%s\" \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/subent.htm \n"
          "  -f <filter>  Filter expression \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/filter.htm \n"
          "     default: \"%s\" \n"
          "  -q <query>  This is just an alias for -f \n"
          " \n"
          " If environment variable SMPCSI is set, it provides the default CSI; \n"
          " otherwise \"%s\" is used. \n"
          " \n"
          , 
          argv[0],
          currentCSI(getenv("SMPCSI")),
          defaultZone,
          defaultEntry,
          defaultSubEntry,
          defaultFilter,
          defaultCSI);
}

static
char* copyAndTrim(char* cursor, size_t maxLength, const char* source){
  size_t i;
  char* start = cursor;
  for (i = 0; i < maxLength; ++i){
    if ( *source == '\0'){
      break;
    }
    *cursor++ = *source++;
  }
  *cursor = 0;

  /* Now work backwards and trim off any blanks. */
  while (--cursor > start){
    if (*cursor != ' '){
      return cursor+1;
    } else{
      *cursor='\0';
    }
  }

  return cursor;
}


int main(int argc, char**argv){
  if (argc == 1){
    usage(argc, argv);
    return 1;
  }
  opterr = 0; /* disable auto error reporting */
  char opt = 0;
  /* These copies are needed because optind and optarg aren't
     necessarily visible to debuggers, and you often want them. */
  int myoptind = 1;
  char* myoptarg = 0;
  bool verbose = false;
  bool debug = false;
  bool messages = false;
  bool header = false;
  char* csi = (char*) currentCSI(getenv("SMPCSI"));
  char* zone = (char*) defaultZone;
  char* entry = (char*) defaultEntry;
  char* subentry = (char*) defaultSubEntry;
  char* filter = 0;

  while (((char) -1) != (opt = (char) getopt(argc, argv, "c:z:e:s:f:q:vmhHd"))){
    myoptind = optind;
    myoptarg = optarg;

    switch(opt){

    case 'c':
      csi = (char*) currentCSI(myoptarg);
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
    case 'q':
      if (filter){
        fprintf(stderr, "filter/query already provided.\n");
        usage(argc, argv);
        return 1;
      }
      filter = myoptarg;
      break;

    case 'v':
      verbose = true;
      messages = true;
      break;

    case 'd':
      debug = true;
      verbose = true;
      messages = true;
      break;

    case 'm':
      messages = true;
      break;

    case 'H':
      header = true;
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
    usage(argc, argv);
    return 1;
  }

  if (0 == filter){
    filter = (char*) defaultFilter;
  }

  APIPGM * gimapi;
  gimapi = (APIPGM *) fetch("GIMAPI");
  if (debug){
    fprintf(stderr, "GIMAPI at 0x%08x\n", (uint32_t) gimapi);
  }

  ITEM_LIST* msgbuff = 0;
  int rc = 0, cc = 0;
  
  if (debug){
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
  P_ENTRY_LIST entrylist = 0;
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

  if (verbose){
    fprintf(stderr, "CSI: %s \n",
            csi);
  }

  (*gimapi) ("QUERY   ", &parmptr, &entrylist, "ENU", &rc, &cc, &msgbuff);
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

  char databuf[1000];
  if (header){
    printf("ENTRY TYPE|ZONE|ENTRY NAME|SUBTYPE|SUBTYPE VALUE|VER SUBTYPE|VER SUBTYPE VALUE\n");
  }
  for (; entrylist !=0 ; entrylist=entrylist->next){
    memset(databuf, 0, sizeof(databuf));
    char* cursor = copyAndTrim(databuf, sizeof(entrylist->type), entrylist->type);
    P_CSI_ENTRY curentry = entrylist->entries;
    if (curentry == 0){
      *cursor = '\0';
      printf("%s\n", databuf);
      continue;
    }

    *cursor++ = '|';
    char* entryCursor = cursor;
    for (; curentry!=0; curentry=curentry->next){
      char* cursor = copyAndTrim(entryCursor, sizeof(curentry->zonename), curentry->zonename);
      *cursor++ = '|';
      cursor = copyAndTrim(cursor, sizeof(curentry->entryname), curentry->entryname);
      P_SUBENTRY topsubentry = curentry->subentries;
      if (topsubentry == 0){
        *cursor = '\0';
        printf("%s\n", databuf);
        continue;
      }

      *cursor++ = '|';
      char* subEntryCursor = cursor;
      for (; topsubentry!=0; topsubentry=topsubentry->next){
        char* cursor = copyAndTrim(subEntryCursor, sizeof(topsubentry->type), topsubentry->type);
        if (topsubentry->subentrydata == 0){
          *cursor = '\0';
          printf("%s\n", databuf);
          continue;
        }
        /* It's got subentries, either VER or normal */
        *cursor++ = '|';
        char* topSubEntryCursor = cursor;

        if (0 == memcmp(topsubentry->type, "VER", 3)){
          /* It's a VER subentry, which has its own list of subitems */
          P_VER verentry = (P_VER)  (topsubentry->subentrydata);
          if (verentry->verdata == 0){
            /* Seem unlikely; why would this even be here? */
            *cursor = '\0';
            printf("%s\n", databuf);
            continue;
          }
          for (; verentry!=0; verentry=verentry->next){
            P_SUBENTRY versubentry = verentry->verdata;
            char* cursor = copyAndTrim(topSubEntryCursor, sizeof(verentry->vernum), verentry->vernum);
            if (versubentry == 0){
              *cursor = '\0';
              printf("%s\n", databuf);
              continue;
            }

            *cursor++ = '|';
            char* verCursor = cursor;
            
            for (; versubentry!=0; versubentry=versubentry->next){
              char* cursor = copyAndTrim(verCursor, sizeof(versubentry->type), versubentry->type);
              if (versubentry->subentrydata == 0){
                *cursor = '\0';
                printf("%s\n", databuf);
                continue;
              }

              *cursor++ = '|';
              char* verSubEntryCursor = cursor;

              P_ITEM_LIST curitem = (P_ITEM_LIST) (versubentry->subentrydata);
              for (; curitem!= 0; curitem = curitem->next){
                char* cursor = copyAndTrim(verSubEntryCursor, (size_t)(curitem->datalen), curitem->data);
                *cursor = '\0';
                printf("%s\n", databuf);
              }
            }
          }
        } else{
          /* It's just a list of items */
          P_ITEM_LIST curitem = (P_ITEM_LIST) (topsubentry->subentrydata);
          for (; curitem!= 0; curitem = curitem->next){
            char* cursor = copyAndTrim(topSubEntryCursor, (size_t)(curitem->datalen), curitem->data);
            *cursor = '\0';
            printf("%s\n", databuf);
          }
        }
      }
    }
  }

  (*gimapi) ("FREE    ", 0, 0, "ENU", &rc, &cc, &msgbuff);

  release((CFUNC*) gimapi);
  return 0;
}
