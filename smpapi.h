#ifndef _SMPAPI_H_
#define _SMPAPI_H_

typedef void APIPGM();
#pragma linkage(APIPGM,OS)
typedef void CFUNC();

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

#endif /* _SMPAPI_H_ */
