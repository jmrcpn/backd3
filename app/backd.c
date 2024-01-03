// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Daemon which handle backup			*/
/*							*/
/********************************************************/
#include	<stdlib.h>
#include	<string.h>
#include	<syslog.h>
#include	<uuid/uuid.h>

#include	"gestap.h"
#include	"subrou.h"

#define	BLKSIZE		32768

#define	SENDMSG		"sendmsg.sh"

#define	SIGPROBE	0	//send signal 0 to a process

typedef	enum {		/*status type			*/
	s_ok,		/*backup successfully terminated*/
	s_notape,	/*no tape avaliable		*/
	s_notinserted,	/*tape not in tape reader.	*/
	s_tapein,	/*tape ready and inserted	*/
	s_noslot,	/*no slot found on tape		*/
	s_toobig,	/*data volume too big for tape	*/
	s_wrongtape,	/*tape ready and inserted	*/
	s_unknown	/*unknown error			*/
	}STATYPE;

/*

*/
/********************************************************/
/*							*/
/*	marker; this procedure stamp the tape with	*/
/*	a label.					*/
/*							*/
/*	Parameters list					*/
/*	[-r debug-root]					*/
/*	label						*/
/********************************************************/
int marker(int argc,char *argv[])

{
#define OPEP    "backd.c:marker,"

//acceptable argument list
const char *arg="b:d:hr:u:";	

//local parameters
int status;
TAPTYP *tape;
LSTTYP **list;
ARGTYP *params;
uuid_t uuid;
register int phase;
register int proceed;

status=-1;
(void) uuid_generate(uuid);
verbose=true;
tape=(TAPTYP *)0;
list=(LSTTYP **)0;
params=(ARGTYP *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0	:	//extracting the parameters
      if ((params=par_getparams(argc,argv,arg))==(ARGTYP *)0) {
	(void) par_usage(argv[0],wha_marker);
	(void) rou_alert(0,"%s Bad argument list", OPEP);
	phase=999;	//not going further
        }
      break;
    case 1	:	//Preparing tape structure
      if ((tape=tap_newtap("none",uuid,params))==(TAPTYP *)0) {
	(void) rou_alert(0,"%s Unable to assign tape structure (system Bug?)",
                            OPEP);
	phase=999;	//not going further
        }
      break;
    case 2	:	//getting the current tape list
      if ((list=tap_readtapefile((char *)0))==(LSTTYP **)0) {
	(void) rou_alert(0,"%s Unable to get current tape liste (config missing?)",
                            OPEP);
	phase=999;	//not going further
        }
      break;
    case 3	:	//is the tape ID already existing?
      break;
    case 4	:	//everything right
      int i;
      ETATYP togo;

      status=0;
      for (i=0;(i<params->argc)&&(status==0);i++) {
        (void) strncpy(tape->id[0],params->argv[i],sizeof(tape->id[0])-1);
	togo=tap_initheader(tape);
        switch(togo) {
          case tap_ok           :       //everything fine, lets continue
            (void) tap_writetapefile((const char *)0,list);
            (void) rou_alert(0,"Unit <%s> set with label <%s>",
                                params->device,tape->id[0]);
            break;
          case tap_nodevice     :       //device missing
            status=-1;
            break;
          case tap_wronguuid    :       //previous tape stil on line
            char goahead;

            goahead=' ';
            (void) fprintf(stdout,"Please change tape on Unit <%s> "
                                  "and press enter (CTRL-C to abort)\n",
                                  params->device);
            while (goahead!='\n') 
              goahead=fgetc(stdin);       
            i--;                        //looping on same tape
            break;
          default               :
            (void) rou_alert(0,"unable to write id <%s> on tape Unit <%s>",
                                tape->id[0],params->device);
            status=-1;
            break;
	  }
	}
      tape=tap_freetape(tape);
      list=(LSTTYP **)rou_freelist((void **)list,(freehandler_t)tap_freeentry);
      break;
    default	:	//exiting procedure
      proceed=false;
      break;
    }
  phase++;
  }
params=par_freeparams(params);
if (rootdir!=(char *)0)
  (void) free(rootdir);
return status;
#undef  OPEP
}
/*

*/
/********************************************************/
/*							*/
/*	Main routine					*/
/*		dispatch action according program name	*/
/*							*/
/********************************************************/
int main(int argc,char *argv[])

{
int status;
char *ptr;
char *name;

status=0;
(void) rou_opensubrou();
(void) tap_opengestap();
if ((ptr=strrchr(argv[0],'/'))!=(char *)0) 
   ptr++;
else
   ptr=argv[0];
name=strdup(ptr);
(void) openlog(name,LOG_PID,LOG_DAEMON);
switch (par_whatami(name)) {
  case wha_marker	:		//mark the tap with a label
    status=marker(argc,argv);
    break;
  default		:
    (void) rou_alert(0,"'%s' program name is unexpected (bug?)",name);
    break;
  }
(void) closelog();
if (status!=0) {
  (void) fprintf(stderr,"the '%s' command was not successfull "
                        "(error code='%d')\n",name,status);
  }
(void) free(name);
(void) tap_opengestap();
(void) rou_opensubrou();
return status;
}
