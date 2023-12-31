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
      char id[50];
      char data[50];

      ptr=(char *)header;
      while (sscanf(ptr,"%[^:]:%[^\n\r]\n%n",id,data,&num)==2) {
        ptr+=num;
        if ((tape=dispatchid(tape,id,data))==(TAPTYP *)0) {
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
#define OPEP    "unitap.c:tapetostr,"

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
