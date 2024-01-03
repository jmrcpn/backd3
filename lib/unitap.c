// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Low level subroutine implementation	        */
/*	handle tape contents                            */
/*							*/
/********************************************************/
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>

#include	"subrou.h"
#include	"unitap.h"

const struct    {
        const   char *id;       //header entry name
        u_int   num;            //header number
        }hdrlst[]=
          {
          {"TapeId",1},
          {"PreviousID",2},
          {"NextId",3},
          {"UUID",4},
          {(const char *)0,0}
          };

static  int modopen;            //boolean module open/close
/*
^L
*/
/********************************************************/
/*							*/
/*	procedure to add data to the tape structure     */
/*							*/
/********************************************************/
static TAPTYP *dispatchid(TAPTYP *tape,char *id,char *data)

{
#define OPEP    "unitape.c:dispatchid,"

int i;

for (i=0;(hdrlst[i].id!=(const char *)0)&&(tape!=(TAPTYP *)0);i++) {
  if (strcmp(hdrlst[i].id,id)==0) {
    int max;
    char *ptr;

    max=0; 
    ptr=(char *)0;
    switch (i) {
      case 0    :
      case 1    :
      case 2    :
        max=sizeof(tape->id[i])-1;
        ptr=tape->id[i];
        break;
      case 3    : 
        max=sizeof(tape->uuid)-1;
        ptr=tape->uuid;
        break;
      default   :
        break;
      }
    if ((ptr==(char *)0)||(strlen(ptr)>0)) {
      (void) rou_alert(3,"%s Unable to add id <%s> within "
                         "tape structure (header format?)",OPEP,id);
      tape=tap_freetape(tape);
      }
    else 
      (void) strncpy(ptr,data,max);
    break;
    }
  }
return tape;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*	procedure to free tap structure.                */
/*							*/
/********************************************************/
TAPTYP *tap_freetape(TAPTYP *data)

{
if (data!=(TAPTYP *)0) {
  if (data->device!=(char *)0)
    (void) free(data->device);
  (void) free(data);
  }
data=(TAPTYP *)0;
return data;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Convert a tape structure to a string (to be used*/
/*      within tapeliste).                              */
/*							*/
/********************************************************/
char *tap_tapetostr(TAPTYP *tape)

{
char *line;

line=(char *)0;
if (tape!=(TAPTYP *)0) {
  char data[300];

  (void) snprintf(data,sizeof(data)-5,"%-16s%-7s"
                                      "% 5d% 12ld% 13d% 6d% 10d",
                                      tape->id[0],
                                      tape->pool,
                                      tape->blocks,
                                      tape->lastused,
                                      tape->frozen,
                                      tape->cycled,
                                      tape->pidlock);
  line=strdup(data);
  }
return line;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Convert a string (from tapeliste) to a tape     */
/*      structure.                                      */
/*							*/
/********************************************************/
TAPTYP *tap_strtotape(const char *data)

{
TAPTYP *tape;

tape=(TAPTYP *)0;
if (data!=(const char *)0) {
  u_long blocks;
  u_int lastused;
  u_int frozen;
  u_int cycled;
  u_int pidlock;
  char *id;
  char *pool;

  blocks=0;
  lastused=0;
  frozen=0;
  cycled=0;
  pidlock=0;
  id=(char *)0;
  pool=(char *)0;
  if (sscanf(data,"%m[a-z,A-Z-,0-9]"    //tapeid
                  " %m[a-z,A-Z-,0-9]"   //poolid
                  " %ld"                //blocksize
                  " %u"                 //lastused
                  " %u"                 //frozen
                  " %u"                 //cycled
                  " %u"                 //pidlock
                 ,&id,&pool,&blocks,      //minimal information
                 &lastused,&frozen,&cycled,&pidlock)>=3) {
    tape=(TAPTYP *)calloc(1,sizeof(TAPTYP));
    if (id!=(char *)0) {
      (void) strncpy(tape->id[0],id,sizeof(tape->id[0])-1);
      (void) free(id);
      }
    if (pool!=(char *)0) {
      (void) strncpy(tape->pool,pool,sizeof(tape->pool)-1);
      (void) free(pool);
      }
    tape->blocks=blocks;
    tape->lastused=lastused;
    tape->frozen=frozen;
    tape->cycled=cycled;
    tape->pidlock=pidlock;
    }
  }
return tape;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to convert a tape header to a TAPTYP  */
/*      structure.                                      */
/*							*/
/********************************************************/
TAPTYP *tap_headertotape(void *header)

{
#define OPEP    "unitape.c:tap_strtotape,"

TAPTYP *tape;
int phase;
int proceed;

tape=(TAPTYP *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :       //do we have an header?
      if (header==(void *)0) {
        (void) rou_alert(2,"%s tape header missing <%s> (Bug?)",OPEP);
        phase=999;      //no need to go further
        }
      break;
    case 1      :       //preparing the tape area
      tape=(TAPTYP *)calloc(1,sizeof(TAPTYP));
      break;
    case 2      :       //do we have an header?
      int num;
      char *ptr;
      char *data;
      char id[20];

      data=(char *)0;
      ptr=(char *)header;
      while (sscanf(ptr,"%" "sizeof(id)-1" "[^:]:"
                        "%m[^\n\r]"
                        "\n%n",id,&data,&num)==2) {
        ptr+=num;
        tape=dispatchid(tape,id,data);
        (void) free(data);
        data=(char *)0;
        if (tape==(TAPTYP *)0) {
          phase=999;            //no need to go further
          break;
          }
        }
      break;
    default     :
      proceed=false;
      break;
    }
  phase++;
  }
return tape;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to convert a TAPTYP to a string       */
/*      within a area of blocksize                      */
/*							*/
/********************************************************/
void *tap_tapetoheader(TAPTYP *tape)

{
#define OPEP    "unitap.c:tapetoheader,"

void *zone;
int phase;
int proceed;

zone=(void *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :
      if (tape==(TAPTYP *)0) {
        (void) rou_alert(2,"%s tape structure missing (Bug?)",OPEP);
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //converting header to string
      char *ptr;
      int i;

      zone=(void *)calloc(1,tape->blksize);
      ptr=(char *)zone;
      for (i=0;hdrlst[i].id!=(const char *)0;i++) {
        char * data;

        data="???";
        switch(i)  {
          case 0        :
          case 1        :
          case 2        :
            data=tape->id[i];
            break;
          case 3        :
            data=tape->uuid;
            break;
          default       :
            (void) rou_alert(0,"%s Unexpected header filed <%s> (Bug?)",
                                   OPEP,hdrlst[i].id);
            break;
          }
        ptr+=sprintf(ptr,"%s:%s\n",hdrlst[i].id,data);
        }
      break;
    default     :
      proceed=false;
      break;
    }
  phase++;
  }
return zone;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*	Procedure to "open" usager to module.           */
/*      homework purpose                                */
/*      return zero if everything right                 */
/*                                                      */
/********************************************************/
int tap_openunitap()

{
int status;

status=0;
if (modopen==false) {
  (void) rou_opensubrou();
  modopen=true;
  }
return status;
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*	Procedure to "close" usager to module.          */
/*      homework purpose                                */
/*      return zero if everything right                 */
/*                                                      */
/********************************************************/
int tap_closeunitap()

{
int status;

status=0;
if (modopen==true) {
  (void) rou_closesubrou();
  modopen=false;
  }
return status;
}
