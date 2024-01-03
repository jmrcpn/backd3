// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Low level subroutine declaration	        */
/*	to hanle an argv list and extract	        */
/*	parameters.				        */
/*							*/
/********************************************************/
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>

#include	"subrou.h"
#include	"unipar.h"

typedef struct {
	const char *id;		//application name
	WHATTYPE what;		//what
	}LSTTYP;

const LSTTYP app_name[]={
	  {"backd",wha_backd},
	  {"backd-marker",wha_marker},
	  {"backd-reader",wha_reader},
	  {"backd-recover",wha_recover},
	  {(const char *)0,wha_unknown}
	  };

static  int modopen;            //boolean module open/close
/*
^L
*/
/********************************************************/
/*							*/
/*						        */
/*	Procedure to init the argument list	        */
/*							*/
/********************************************************/
static ARGTYP *initparams()

{
ARGTYP *params;

params=(ARGTYP *)calloc(1,sizeof(ARGTYP));
params->argv=(char **)0;
params->device=(char *)0;
params->pool=(char *)0;
params->blksize=0;
return params;
}
/*
^L
*/
/********************************************************/
/*							*/
/*	Procedure to free memory used by an arg         */
/*	list					        */
/*							*/
/********************************************************/
ARGTYP *par_freeparams(ARGTYP *params)

{
if (params!=(ARGTYP *)0) {
  if (params->argv!=(char **)0) {
    char **ptr;

    ptr=params->argv;
    while (*ptr!=(char *)0) {
      (void) free(*ptr);
      ptr++;
      }
    (void) free(params->argv);
    }  
  if (params->pool!=(char *)0)
    (void) free(params->pool);
  if (params->device!=(char *)0)
    (void) free(params->device);
  (void) free(params);
  params=(ARGTYP *)0;
  }
return params;
}
/*
^L
*/
/********************************************************/
/*							*/
/*	Procedure to extract argument list	        */
/*	but cherry-pick only the one allowed by         */
/*	sequence optstring.			        */
/*	on error return an null argtype		        */
/*							*/
/********************************************************/
ARGTYP *par_getparams(int argc,char * const argv[],const char *optstring)

{
ARGTYP *params;
int c;

params=initparams();
while (((c=getopt(argc,argv,optstring))!=EOF)&&(params!=(ARGTYP *)0)) {
  switch(c) {
    case 'b'	:       //block size
      params->blksize=atoll(optarg);
      break;
    case 'd'	:       //debug level
      debug=atoi(optarg);
      (void) rou_alert(1,"debug level is now '%d'",debug);
      break;
    case 'h'	:               //requestion program help
      params=par_freeparams(params);
      break;
    case 'p'	:       //pool name
      if (params->pool!=(char *)0)
        (void) free(params->pool);
      params->pool=strdup(optarg);
      break;
    case 'r'	:
      if (rootdir!=(char *)0)
        (void) free(rootdir);
      rootdir=strdup(optarg);
      break;
    case 'u'	:
      (void) free(params->device);
      params->device=strdup(optarg);
      break;
    case 'v'	:
      verbose=true;
      (void) rou_alert(0,"application run in verbose mode");
      break;
    default	:
      (void) rou_alert(0,"\"%s\" unexpected argument designator",argv[optind-1]);
      params=par_freeparams(params);
      break;
    }
  }
if ((params!=(ARGTYP *)0)&&(argc>optind)) {
  int i;

  params->argc=argc-optind;
  params->argv=(char **)calloc(params->argc+1,sizeof(char *));
  for (i=0;i<params->argc;i++) {
    params->argv[i]=strdup(argv[optind+i]);
    }
  }
return params;
}
/*

*/
/********************************************************/
/*							*/
/*	lets find out what is the backd impersonation 	*/
/*	according backd called name, function is	*/
/*	sligthly different.				*/
/*							*/
/********************************************************/
WHATTYPE par_whatami(char *myname)

{
WHATTYPE result;
const LSTTYP *ptr;
char *name;

result=wha_unknown;
ptr=app_name;
if ((name=strrchr(myname,'/'))!=(char *)0)
  name++;
else
  name=myname;
while (ptr->id!=(char *)0) {
  if (strcmp(name,ptr->id)==0) {
    result=ptr->what;
    break;
    }
  ptr++;
  }
return result;
}
/*

*/
/********************************************************/
/*							*/
/*	Display a list of parameters available according*/
/*	the name used to call the backd program		*/
/*							*/
/********************************************************/
int par_usage(char *name,WHATTYPE called)

{
int status;

status=0;
(void) fprintf(stderr,"%s Version <%s>\n",name,rou_getversion());
(void) fprintf(stderr,"usage:\n  ");
switch (called) {
  case wha_marker	:
    (void) fprintf(stderr,"%s\t"
		          "[-b blocksize] "
		          "[-d debug] "
		          "[-h] "
		          "[-p poolname] "
		          "[-r root] "
			  "-u unit "
			  "tapename\n",
		    	  app_name[(int)called].id);
    break;
  case wha_recover	:
    (void) fprintf(stderr,"\t\t-v\t: verbose mode\n");
    break;
  default		:
    status=-1;
    (void) rou_alert(0,"'%d' unexpected called name (bug?)",(int)called);
    break;
  }
//common parameters list
if (status==0) {
  (void) fprintf(stderr,"\twhere:\n");
  (void) fprintf(stderr,"\t\t-b blks : tape block size\n");
  (void) fprintf(stderr,"\t\t-d level: debug level [1-10]\n");
  (void) fprintf(stderr,"\t\t-h\t: print this help message\n");
  (void) fprintf(stderr,"\t\t-r root\t: root working directory\n");
  (void) fprintf(stderr,"\t\t-u unit\t: storage device name\n");
  switch (called) {
    case wha_marker	:
      (void) fprintf(stderr,"\t\ttapename: Label to write in the tape header\n");
      break;
    default		:
      break;
    }
  }
return status;
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
int par_openunipar()

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
int par_closeunipar()

{
int status;

status=0;
if (modopen==true) {
  (void) rou_closesubrou();
  modopen=false;
  }
return status;
}
