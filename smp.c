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
          "     * (default) - all entry types \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/entry.htm \n"
          "  -s <subtype(s)>  Subtypes() of entries to be displayed \n"
          "     * (default) - all subtypes types \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/subent.htm \n"
          "  -f <filter>  Filter expression \n"
          "     See: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.gim2000/filter.htm \n"
          " \n"
          , 
          argv[0]);
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
