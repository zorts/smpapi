#ifndef _SMPAPI_H_
#define _SMPAPI_H_

#if defined(__MVS__) && (defined (__IBMC__) || defined (__IBMCPP__))
/* Structure packing */
#define RKT_PRAGMA_PACK  _Pragma ( "pack(packed)" )
#define RKT_PRAGMA_PACK_RESET  _Pragma ( "pack(reset)" )
#endif

typedef void APIPGM();
#pragma linkage(APIPGM,OS)
typedef void CFUNC();

RKT_PRAGMA_PACK
typedef struct QUERY_PARMS
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

typedef struct API_Version
{
  char apiver[2];
  char apirel[2];
  char apimod[2];
  char apiptf[2];
} API_VERSION, * P_API_VERSION;

typedef struct CSI_ENTRY
{
  struct CSI_ENTRY *next;
  struct SUBENTRY *subentries;
  char entryname[8];
  char zonename[7];
} CSI_ENTRY, * P_CSI_ENTRY;

typedef struct ENTRY_LIST
{
  struct ENTRY_LIST *next;
  struct CSI_ENTRY *entries;
  char type[12];
} ENTRY_LIST, * P_ENTRY_LIST;

typedef struct ITEM_LIST
{
  struct ITEM_LIST *next;
  long datalen;
  char *data;
} ITEM_LIST, * P_ITEM_LIST;


typedef struct SUBENTRY
{
  struct SUBENTRY *next;
  void *subentrydata;
  char type[12];
} SUBENTRY, * P_SUBENTRY;

typedef struct VER
{
  struct VER *next;
  struct SUBENTRY *verdata;
  char vernum[3];
} VER, * P_VER;

RKT_PRAGMA_PACK_RESET

#endif /* _SMPAPI_H_ */
