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

#define TAPES   "/etc/backd/tapelist"

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

*/
/********************************************************/
/*							*/
/*	Subroutine to generate a tape definition entry, */
/*      discriminate comment part from the datapart     */
/*      within the line                                 */
/*							*/
/********************************************************/
static LSTTYP *gettapeinfo(char *str)

{
LSTTYP *entry;
register char *comment;

entry=(LSTTYP *)calloc(1,sizeof(LSTTYP));
if ((comment=strchr(str,'#'))!=(char *)0) {
  *comment='\000';
  comment++;
  }
if (strlen(str)>0)
  entry->tapedata=tap_strtotape(str);
if (comment!=(char *)0)
  entry->comment=strdup(comment);
return entry;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to scan a tapelist file keeping       */
/*      comments and parsing tape description if needed */
/*      return a tape structure                         */
/*							*/
/********************************************************/
static LSTTYP **scanliste(FILE *fichier)

{
LSTTYP **list;
char line[3000];

list=(LSTTYP **)0;
while (rou_getstr(fichier,line,sizeof(line)-1,true,'#')!=(char *)0) {
  LSTTYP *entry;

  if ((entry=gettapeinfo(line))!=(LSTTYP *)0) {
    list=(LSTTYP **)rou_addlist((void **)list,(void *)entry);
    }
  }
return list;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to free all memory used by one tape   */
/*      list entry.                                     */
/*							*/
/********************************************************/
LSTTYP *tap_freeentry(LSTTYP *entry)

{
if (entry!=(LSTTYP *)0) {
  entry->tapedata=tap_freetape(entry->tapedata);
  if (entry->comment!=(char *)0)
    (void) free(entry->comment);
  (void) free(entry);
  entry=(LSTTYP *)0;
  }
return entry;
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to lock exclusive access to tape list.*/
/*      return lock status.                             */
/*							*/
/********************************************************/
int tap_locktapelist(const char *filename,int lock,int tentative)

{
if (filename==(char *)0) 
  filename=TAPES"_lock";
return rou_locking(filename,lock,tentative);
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to add a tape definition to a         */
/*      tapeliste.                                      */
/*							*/
/********************************************************/
LSTTYP **tap_addtolist(LSTTYP **list,TAPTYP *tape)

{
if (tape!=(TAPTYP *)0) {
  LSTTYP *toadd;
  char comment[50];

  toadd=(LSTTYP *)calloc(1,sizeof(LSTTYP));
  (void) sprintf(comment,"Label Stamp: %s",rou_getstrfulldate(time((time_t *)0)));
  toadd->comment=strdup(comment);
  toadd->tapedata=(TAPTYP *)calloc(1,sizeof(TAPTYP));
  (void) memcpy(toadd->tapedata,tape,sizeof(TAPTYP));
  list=(LSTTYP **)rou_addlist((void **)list,(void *)toadd);
  }
return list;
}
/*
^L
*/
/********************************************************/
/*							*/
/*	procedure to initiat a clean tap structure.     */
/*							*/
/********************************************************/
TAPTYP *tap_inittape(char *label,uuid_t uuid,ARGTYP *params)

{
TAPTYP *tape;

tape=tap_newtape();
(void) uuid_unparse_upper(uuid,tape->uuid);
if (params->device!=(char *)0)
  (void) strncpy(tape->device,params->device,sizeof(tape->device));
if (params->pool!=(char *)0) 
  (void) strncpy(tape->pool,params->pool,sizeof(tape->pool));
if (params->blksize>0)
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

      oldtape=tap_readheader(tape->device,tape->blksize);
      if ((oldtape!=(TAPTYP *)0)&&(strcmp(oldtape->uuid,tape->uuid)==0)) {
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
    case 4      :               //writing|updating the tape device
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
        (void) rou_alert(1,"%s tape device incomplete (bug?)",OPEP);
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
/*							*/
/*      Procedure to build a tape list extracted from   */
/*      file configuration list                         */
/*							*/
/********************************************************/
LSTTYP **tap_readtapefile(const char *filename)

{
#define OPEP    "gestap.c:tap_readtapefile,"

LSTTYP **list;
char *fname;
FILE *fichier;
int phase;
int proceed;

list=(LSTTYP **)0;
fname=(char *)0;
fichier=(FILE *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :               //building filename
      if (filename==(char *)0)
        filename=TAPES;
      if ((fname=rou_apppath(filename))==(char *)0) {
        (void) rou_alert(1,"%s Unable to get the tapelist filename (bug?)",OPEP);
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //Is the file existing
      if ((fichier=fopen(fname,"r"))==(FILE *)0) {
        (void) rou_alert(0,"%s Unable to read tape list <%s> (error=<%s>)",
                            OPEP,fname,strerror(errno));
        phase=999;              //no need to go further
        }
      break;
    case 2      :               //building list
      if ((list=scanliste(fichier))==(LSTTYP **)0)
        (void) rou_alert(1,"%s Unable to scan tape list <%s> (config format?)",
                            OPEP,fname);
      break;
    case 3      :               //closing file
      (void) fclose(fichier);
      (void) free(fname);
      break;
    default     :               //exiting
      proceed=false;
      break;
    }
  phase++;
  }
return list;
#undef  OPEP
}
/*
^L
*/
/********************************************************/
/*							*/
/*      Procedure to write of update a tapelist file    */
/*      with list of tape contents                      */
/*							*/
/********************************************************/
int tap_writetapefile(const char *filename,LSTTYP **list)

{
#define OPEP    "gestap.c:tap_witetapefile,"

int status;
char *fname;
FILE *fichier;
int phase;
int proceed;

status=-1;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0      :               //building filename
      if (filename==(char *)0)
        filename=TAPES;
      if ((fname=rou_apppath(filename))==(char *)0) {
        (void) rou_alert(1,"%s Unable to get the tapelist filename (bug?)",OPEP);
        phase=999;              //no need to go further
        }
      break;
    case 1      :               //removing previous list
      if (fname!=(char *)0) {   //always
        char *old;

        old=(char *)calloc(strlen(fname)+10,sizeof(char));
        (void) strcat(old,fname);
        (void) strcat(old,".prv");
        (void) unlink(old);     //remove previous tapelist
        if (link(fname,old)<0) {
          (void) rou_alert(0,"%s Unable to link file <%s> and <%s> "
                             "(bug?, error=<%s>)",
                              OPEP,fname,old,strerror(errno));
          phase=999;            //no need to go further
          }
        else
          (void) unlink(fname);
        (void) free(old);
        }
      break;
    case 2      :               //opening the file in writing mode
      if ((fichier=fopen(fname,"w"))<0) {
        (void) rou_alert(0,"%s Unable to open file <%s> (error=<%s>)",
                              OPEP,fname,strerror(errno));
        phase=999;              //no need to go further
        }
      break;
    case 3      :               //dumping the list content on file
      if (list!=(LSTTYP **)0) {
        register int num;

        num=0;
        while (list[num]!=(LSTTYP *)0) {
          char *data;

          if ((data=tap_tapetostr(list[num]->tapedata))!=(char *)0) {
            (void) fprintf(fichier,"%s\t",data);
            (void) free(data);
            }
          if (list[num]->comment!=(char *)0) 
            (void) fprintf(fichier,"#%s",list[num]->comment);
          (void) fprintf(fichier,"\n");
          num++;
          }
        }
      break;
    case 4      :               //closing opened tapelist file
      (void) fclose(fichier);
      status=0;
      break;
    default     :
      if (fname!=(char *)0)
        (void) free(fname);
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
/*      Procedure to return a tape reference extracted  */
/*      from a list of tape reference, selection is done*/
/*      according tape label.                           */
/*							*/
/********************************************************/
const TAPTYP *tap_findtape(LSTTYP **list,const char *label)

{
const TAPTYP *tape;

tape=(const TAPTYP *)0;
if ((list!=(LSTTYP **)0)&&(label!=(const char *)0)) {
  while (*list!=(LSTTYP *)0) {
    if ((*list)->tapedata!=(TAPTYP *)0) {
      if (strcmp((*list)->tapedata->id[0],label)==0) {
        tape=(*list)->tapedata;
        break;
        }
      }
    list++;
    }
  }
return tape;
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
