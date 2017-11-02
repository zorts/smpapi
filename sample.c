#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smpapi.h"
#define FREE  "FREE    "
#define QUERY "QUERY   "
#define APILANG "ENU"
#define TXT_VER "VER"
#define LEN_ETYPE 12
#define LEN_ENAME 8
#define LEN_ZNAME 7
#define LEN_VERNUM 3
#define LEN_TXTVER 3
#define LEN_MSG 256

static void errprint(char *, long, long, ITEM_LIST *);
static void valprint(ITEM_LIST *);
static void resprint(ENTRY_LIST *);
void main(int argc, char *argv[])
{
  long rc,cc;
  QUERY_PARMS qparms;
  P_QUERY_PARMS pqparms = &qparms;
  ENTRY_LIST *qreslt;
  ITEM_LIST *msgbuff;
  APIPGM *gimapi;
  char csi[45];
  char zone[100];
  char ent[100];
  char subent[100];
  char filter[150];
  rc = 0;
  cc = 0;
  /***********************************************/
  /* Load the GIMAPI load module for use later */
  /***********************************************/
  gimapi = (APIPGM *) fetch("GIMAPI");
  /*****************************************************/
  /* Create the QUERY. Put the parameter strings into */
  /* variables and put the addresses of those variables*/
  /* in the query parameter structure along with the */
  /* length of those strings. */
  /*****************************************************/
  strcpy(csi,"SMPE.ZOSV201.GLOBAL.CSI");
  strcpy(zone,"ALLTZONES");
  strcpy(ent,"SYSMOD");
  strcpy(subent,"MOD,INSTALLDATE");
#if 0
  strcpy(filter,"(SMODTYPE=PTF | SMODTYPE=USERMOD)");
  strcat(filter," & FMID=HMP1E00 & APPLY=YES");
  strcat(filter," & BYPASS=YES & RECDATE>07335");
#else
  strcpy(filter,"");
#endif
  qparms.csi = csi;
  qparms.csilen = strlen(csi);
  qparms.zone = zone;
  qparms.zonelen = strlen(zone);
  qparms.entrytype = ent;
  qparms.entrylen = strlen(ent);
  qparms.subentrytype = subent;
  qparms.subentrylen = strlen(subent);
  qparms.filter = filter;
  qparms.filterlen = strlen(filter);
  gimapi(QUERY,&pqparms,(void**) &qreslt,APILANG,&rc,&cc,&msgbuff);
  if (rc!=0)
    {
      errprint(QUERY, rc, cc, msgbuff);
      if (rc>4) goto EXIT;
    }
  /******************************************/
  /* Call routine to print results of query */
  /******************************************/
  resprint(qreslt);
  /****************************************/
  /* Free storage returned from the QUERY */
  /****************************************/
  gimapi(FREE,0,0,APILANG,&rc,&cc,&msgbuff);
EXIT:
  release ((CFUNC*)gimapi);
}

/******************************/
/* Print results of the query */
/******************************/
static void resprint(ENTRY_LIST *head)
{
  ENTRY_LIST *curetype;
  CSI_ENTRY *curentry;
  SUBENTRY *cursubent;
  VER *curver;
  SUBENTRY *curversub;
  char etype[13];
  char vernumber[13];
  char versubtype[13];
  char stEname[LEN_ENAME+1];
  char stZname[LEN_ZNAME+1];
  /********************************/
  /* Loop through each entry type */
  /********************************/
  for (curetype=head; curetype!=0 ; curetype=curetype->next)
    {
      /********************************************/
      /* Print name of entry type being processed */
      /********************************************/
      strncpy(etype,curetype->type,LEN_ETYPE);
      etype[LEN_ETYPE] = '\0';
      printf("Entry Type: %s\n",etype);
      /********************************************************/
      /* Loop through each entry printing the ename and zone */
      /* then the list of subentry values. */
      /********************************************************/
      for (curentry=curetype->entries;
           curentry!=0;
           curentry=curentry->next)
        {
          printf("----------------------------------------\n");
          strncpy(stEname,curentry->entryname,LEN_ENAME);
          stEname[LEN_ENAME]='\0';
          strncpy(stZname,curentry->zonename,LEN_ZNAME);
          stZname[LEN_ZNAME]='\0';
          printf(" ENAME : %s\n",stEname);
          printf(" ZONE : %s\n",stZname);
          for (cursubent=curentry->subentries;
               cursubent!=0;
               cursubent=cursubent->next)
            {
              strncpy(etype,cursubent->type,LEN_ETYPE);
              etype[LEN_ETYPE] = '\0';
              if ((strncmp(etype,TXT_VER,LEN_TXTVER)) == 0)
                {
                  for (curver=(P_VER) cursubent->subentrydata;
                       curver!=0;
                       curver=curver->next)
                    {
                      strncpy(vernumber,curver->vernum,LEN_VERNUM);
                      vernumber[LEN_VERNUM]='\0';
                      for (curversub=curver->verdata;
                           curversub!=0;
                           curversub=curversub->next)
                        {
                          /*********************************/
                          /* Now print ver subentry values */
                          /*********************************/
                          strncpy(versubtype,curversub->type,LEN_ETYPE);
                          versubtype[LEN_ETYPE]='\0';
                          printf(" %.6s VER(%s): ",versubtype,vernumber);
                          valprint(curversub->subentrydata);
                        }
                    }
                } /* end ver subentry */
              else /* not a ver structure */
                {
                  printf(" %s : ",etype);
                  valprint(cursubent->subentrydata);
                } /* end non-ver subentry */
            } /* end subentry for */
        } /* end entries for */
    } /* end entry type for */
}

static void valprint(ITEM_LIST *item1)
{
  char databuff[500];
  ITEM_LIST *curitem;
  for (curitem=item1;
       curitem!=0;
       curitem=curitem->next)
    {
      strncpy(databuff,curitem->data,curitem->datalen);
      databuff[curitem->datalen] = '\0';
      printf("%s\n",databuff);
      if (curitem->next!=0)
        printf("\n ");
    } /* end item for */
}

static void errprint(char *cmd, long rc, long cc, ITEM_LIST *msgs)
{
  char msgout[LEN_MSG+1];
  ITEM_LIST *curmsg;
  unsigned short i;
  printf("Error processing command: %s. RC=%d CC=%d\n",
         cmd,rc,cc);
  printf("Messages follow:\n");
  /*************************************************/
  /* Loop through a linked list of error messages */
  /* printing them out. */
  /*************************************************/
  for (curmsg=msgs; curmsg!=0; curmsg=curmsg->next)
    {
      strncpy(msgout,curmsg->data,curmsg->datalen);
      msgout[curmsg->datalen] = '\0';
      printf("%s\n",msgout);
    }
}
