// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Module to manage tap device access              */
/*							*/
/********************************************************/
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>

#include	"subrou.h"
#include	"gestap.h"

static  int modopen;            //boolean module open/close
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to check if the unit (device) is      */
/*      existing.                                       */
/*      Return zero if unit available                   */
/*							*/
/********************************************************/
static int deviceok(char *device)

{
#define OPEP    "gestap.c:deviceok,"

int status;
FILE *ready;
int phase;
int proceed;

status=-1;
ready=(FILE *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :       //do we have a unit
      if (device==(char *)0) {
        (void) rou_alert(2,"%s Unitname (tape device) is unknown!",OPEP);
        phase=999;      //No need to go further
        }
      break;
    case 1      :       //is the unit accessible
      char *tapename;

      tapename=rou_apppath(device);
      if ((ready=fopen(tapename,"rw"))==(FILE *)0) {
        (void) rou_alert(2,"%s Unable to open unit <%s> (error=<%s)",
                           OPEP,tapename,strerror(errno));
        phase=999;      //No need to go further
        }
      (void) free(tapename);
      break;
    case 2      :       //everything is fine, closing file      
      (void) fclose(ready);
      status=0;
      break;
    default     :
      proceed=false;
      break;
    }
  phase++;
  }
return status;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*	procedure to initiat a clean tap structure.     */
/*							*/
/********************************************************/
TAPTYP *tap_newtap(char *label,uuid_t uuid,ARGTYP *params)

{
TAPTYP *tape;

tape=(TAPTYP *)calloc(1,sizeof(TAPTYP));
(void) strncpy(tape->id[0],label,sizeof(tape->id[0])-1);
(void) strcpy(tape->id[1],"none");
(void) strcpy(tape->id[2],"none");
(void) uuid_unparse_upper(uuid,tape->uuid);
if (params->device!=(char *)0)
  tape->device=strdup(params->device);
tape->blksize=params->blksize;
return tape;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to write an header on a "new" tape    */
/*							*/
/********************************************************/
ETATYP tap_initheader(TAPTYP *tape)

{
#define OPEP    "gestap.c:tap_initheader,"
ETATYP status;
char *lockname;
int phase;
int proceed;

status=tap_trouble;
lockname=(char *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :
      if ((tape==(TAPTYP *)0)||(deviceok(tape->device)!=0)) {
        status=tap_nodevice;
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //setting local lockname
      if ((lockname=strrchr(tape->device,'/'))==(char *)0)
        lockname=tape->device;
      else 
        lockname++;
      break;
    case 2      :               //is current tape with the same uuid
      TAPTYP *oldtape;

      oldtape=(tap_readheader(tape->device,tape->blksize));
      if ((oldtape!=(TAPTYP *)0)&&(strcmp(oldtape->uuid,tape->uuid)==0)) {
        (void) rou_alert(0,"Previous Tape with id/uuid <%s/%s>\n"
                           "is still online on device <%s>",
                           oldtape->id[0],tape->uuid,tape->device);
        status=tap_wronguuid;
        phase=999;              //no need to further
        }
      oldtape=tap_freetape(oldtape);
      break;
    case 3      :               //locking device
      if (rou_locking(lockname,LCK_LOCK,10)==false) {
        (void) rou_alert(1,"%s unable to lock device <%s> in due time, ",
                           "another process still running?",OPEP,tape->device);
        status=tap_nolock;
        phase=999;              //no need to further
        }
      break;
    case 4      :               //writing|updating the tape driver
      status=tap_writeheader(tape);
      (void) rou_locking(lockname,LCK_UNLOCK,1);
      break;
    default     :
      proceed=false;
      break;
    }
  phase++;
  }
return status;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to write/update an header on tape     */
/*							*/
/********************************************************/
ETATYP tap_writeheader(TAPTYP *tape)

{
#define OPEP    "gestap.c:tap_writeheader,"

int status;
char *fname;
FILE *ftape;
void *header;
int phase;
int proceed;

status=tap_trouble;
ftape=(FILE *)0;
fname=(char *)0;
header=(unsigned char *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :
      if ((tape==(TAPTYP *)0)||(deviceok(tape->device)!=0)) {
        status=tap_nodevice;
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //converting header to block
      if ((header=tap_tapetoheader(tape))==(void *)0) {
        (void) rou_alert(1,"%s Unable to convert tape header (bug?)",OPEP);
        status=tap_trouble;
        phase=999;              //no need to go further
        }
      break;
    case 2      :               //writing header on device
      fname=rou_apppath(tape->device);
      if ((ftape=fopen(fname,"r+"))==(FILE *)0) {
        (void) rou_alert(1,"%s Unable to open tape device <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        status=tap_nodevice;
        phase=999;              //no need to go further
        }
      break;
    case 3      :               //writing header to tape device
      if (fseek(ftape,0,SEEK_SET)!=0) {
        (void) rou_alert(1,"%s Unable to rewind tape device <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        status=tap_nodevice;
        (void) fclose(ftape);
        phase=999;              //no need to go further
        }
      break;
    case 4      :               //writing header to tape device
      if (fwrite(header,tape->blksize,1,ftape)!=1) {
        (void) rou_alert(1,"%s Unable to write tape device <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        status=tap_nodevice;
        phase=999;              //no need to go further
        }
      (void) fclose(ftape);
      break;
    case 5      :               //success
      status=tap_ok;
      break;
    default     :
      proceed=false;
      break;
    }
  phase++;
  }
if (fname!=(char *)0)
  (void) free(fname);
if (header!=(void *)0)
  (void) free(header);
return status;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to read tape header currently on      */
/*      device                                          */
/*							*/
/********************************************************/
TAPTYP *tap_readheader(char *device,u_i64 blksize)

{
#define OPEP    "gestap.c:tap_readheader,"
TAPTYP *tape;
char *fname;
FILE *ftape;
void *header;
int phase;
int proceed;

tape=(TAPTYP *)0;
fname=(char *)0;
ftape=(FILE *)0;
header=(void *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :               //device ready
      if (device==(char *)0) {
        (void) rou_alert(1,"tape device incomplete (bug?)",OPEP);
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //opening device
      fname=rou_apppath(device);
      if ((ftape=fopen(fname,"r"))==(FILE *)0) {
        (void) rou_alert(1,"%s Unable to open tape device <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        phase=999;              //no need to go further
        }
      break;
    case 2      :               //reading tape header
      header=calloc(1,blksize);
      if (fread(header,blksize,1,ftape)!=1) {
        (void) rou_alert(1,"%s Unable to read tape device <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        phase=999;              //no need to go further
        }
      (void) fclose(ftape);
      (void) free(fname);
      break;
    case 3      :               //converting header to tape
      if ((tape=tap_headertotape(header))==(TAPTYP *)0) {
        (void) rou_alert(1,"%s tape header wrong format",OPEP);
        phase=999;              //no need to go further
        }
      (void) free(header);
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
/*                                                      */
/*	Procedure to "open" usager to module.           */
/*      homework purpose                                */
/*      return zero if everything right                 */
/*                                                      */
/********************************************************/
int tap_opengestap()

{
int status;

status=0;
if (modopen==false) {
  (void) rou_opensubrou();
  (void) par_openunipar();
  (void) tap_openunitap();
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
int tap_closegestap()

{
int status;

status=0;
if (modopen==true) {
  (void) tap_closeunitap();
  (void) par_closeunipar();
  (void) rou_closesubrou();
  modopen=false;
  }
return status;
}
