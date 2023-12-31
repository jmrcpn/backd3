// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Module for low level subroutine			*/
/*							*/
/********************************************************/

#include	<fcntl.h>
#include	<signal.h>
#include	<stdarg.h>
#include        <stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<syslog.h>
#include	<time.h>
#include	<unistd.h>
#include	<sys/stat.h>

#include	"subrou.h"

#define VERSION "3.1"
#define RELEASE "0.9"

#define APPLICATION             "backd"
#define DIRLOCK                 "/var/run/"APPLICATION

/*current debug level					*/
int debug=0;
/*debug log verbosity mode				*/
int verbose=0;
/*backd is started in foreground mode			*/
int foreground=false;

/*Time offset (debug purpose only                       */
time_t off64_time=(time_t)0;
time_t off_date=(time_t)0;

//application specific working directory (debugging)
char *rootdir=(char *)0;


static  int modopen;            //boolean module open/close
/*

*/
/********************************************************/
/*							*/
/*	procedure to allocate a path within the root    */
/*      directory if needed                             */
/*							*/
/********************************************************/
char *rou_apppath(const char *path)

{
char *root;
char *newpath;
int taille;
char loc[3];

root="";
newpath=(char *)0;
if (rootdir!=(char *)0)
  root=rootdir;
taille=snprintf(loc,sizeof(loc),"%s%s",root,path);
newpath=(char *)calloc(taille+3,sizeof(char));
(void) sprintf(newpath,"%s%s",root,path);
return newpath;
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to get the local system time,        */
/*	can be offset by hours or days for	        */
/*	testing purpose.			        */
/*							*/
/********************************************************/
time_t rou_systime()

{
return time((long *)0)+off64_time+off_date;
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to log event on syslog or stderr	*/
/*							*/
/********************************************************/
void rou_alert(const int dlevel,const char *fmt,...)

#define	DEBMAX	80

{
if (debug>=dlevel)
  {
  va_list args;
  char *ptr;
  char strloc[10000];

  va_start(args,fmt);
  (void) vsnprintf(strloc,sizeof(strloc),fmt,args);
  ptr=strloc;
  if (verbose>0)
    (void) fprintf(stderr,"%s\n",strloc);
  else {
    while (strlen(ptr)>DEBMAX) {
      char locline[DEBMAX+10];
  
      (void) strncpy(locline,ptr,DEBMAX);
      locline[DEBMAX]='\000';
      (void) syslog(LOG_INFO,"%s",locline);
      ptr +=DEBMAX;
      } 
    (void) syslog(LOG_INFO,"%s\n",ptr);
    }
  va_end(args);
  }
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to CORE-DUMP the application         */
/*							*/
/********************************************************/
void rou_crash(const char *fmt,...)

{
#define	RELAX	5
va_list args;
char strloc[10000];

va_start(args,fmt);
(void) vsnprintf(strloc,sizeof(strloc),fmt,args);
(void) rou_alert(0,"Crashed on purpose by '%s'",strloc);
(void) rou_alert(0,"Crash delayed by '%d' second",RELAX);
(void) sleep(RELAX);	//To avoid immediat restart
va_end(args);
(void) kill(getpid(),SIGSEGV);
(void) exit(-1);	//unlikely to reach here
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to return the date+time under        */
/*	a string format.			        */
/*							*/
/********************************************************/
char *rou_getstrfulldate(time_t curtime)

{
static char timeloc[50];

struct tm *ttime;

(void) strcpy(timeloc,"");
ttime=localtime(&curtime);
(void) snprintf(timeloc,sizeof(timeloc),
                        "%4d/%02d/%02d-%2d:%02d:%02d",
                        ttime->tm_year+1900,ttime->tm_mon+1,ttime->tm_mday,
			ttime->tm_hour,ttime->tm_min,ttime->tm_sec);
return timeloc;
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to return a time stamp under         */
/*	the AAAAMMJJ format.			        */
/*							*/
/********************************************************/
u_long rou_normdate(time_t timestamp)

{
struct tm *ttime;

ttime=localtime(&timestamp);
return (((u_long)(ttime->tm_year+1900)*10000)+(ttime->tm_mon+1)*100)+ttime->tm_mday; 
}
/*

*/
/********************************************************/
/*							*/
/*	Subroutine to return the date only under        */
/*	a string format.			        */
/*							*/
/********************************************************/
char *rou_getstrdate(u_long normdate)

{
static char dateloc[50];

(void) strcpy(dateloc,"");
(void) snprintf(dateloc,sizeof(dateloc),
                        "%4ld/%02ld/%02ld",
                        normdate/10000,(normdate/100)%100,normdate%100);
return dateloc;
}
/*

*/
/************************************************/
/*						*/
/*	Subroutine to return the time only under*/
/*	a string format.			*/
/*						*/
/************************************************/
char *rou_getstrtime(time_t curtime)

{
static char timeloc[50];

(void) sprintf(timeloc,"%02ld:%02ld:%02ld",curtime/3600,(curtime/60)%60,curtime%60);
return timeloc;
}
/*

*/
/************************************************/
/*						*/
/*	Subroutine to add X days to a time 	*/
/*						*/
/************************************************/
u_long rou_caldate(time_t date,int month,int days)

{
int locmonth;
int annees;
struct tm *ttime;
struct tm memtime;
time_t curdate;

(void) memset((void *)&memtime,'\000',sizeof(memtime));
memtime.tm_mday=date%100;
memtime.tm_mon=((date/100)%100)-1;
memtime.tm_year=(date/10000)-1900;
memtime.tm_hour=12;
annees=abs(month)/12;
locmonth=abs(month)%12;
if (month>0) {
  memtime.tm_year+=annees;
  memtime.tm_mon+=locmonth;
  if (memtime.tm_mon>11) {
    memtime.tm_year++;
    memtime.tm_mon-=12;
    }
  }
if (month<0)
  {
  memtime.tm_year-=annees;
  memtime.tm_mon-=locmonth;
  if (memtime.tm_mon<0) {
    memtime.tm_year--;
    memtime.tm_mon+=12;
    }
  }
curdate=mktime(&memtime);
curdate+=(days*(24*3600));
ttime=localtime(&curdate);
return (((u_long)(ttime->tm_year+1900)*10000)+(ttime->tm_mon+1)*100)+ttime->tm_mday;
}
/*

*/
/********************************************************/
/*							*/
/*	Subroute to read a text file, get rid off the	*/
/*	commented in line (if requested) and return	*/
/*	the meaningful texte.				*/
/*							*/
/********************************************************/
char *rou_getstr(FILE *fichier,char *str,int taille,int comment,char carcom)

{
char *strloc;

while ((strloc=fgets(str,taille,fichier))!=(char *)0) {
  char *ptrloc;

  if (comment==false) {
    if (str[0]==carcom)
      continue;
    }
  ptrloc=str;
  while ((ptrloc=strchr(ptrloc,carcom))!=(char *)0) {
    if (*(ptrloc-1)=='\\') {
      (void) strcpy(ptrloc-1,ptrloc);	
      ptrloc++;
      }
    else {
      if (comment==false)
        *ptrloc='\000';
      break;
      }
    }
  if (str[strlen(str)-1]=='\n')
    str[strlen(str)-1]='\000';
  break;
  }
return strloc;
}
/*

*/
/********************************************************/
/*                                                      */
/*	Subroutine to return a time expressed	        */
/*	as a local date				        */
/*						        */
/********************************************************/
char *rou_localedate(time_t date)

{
char *str;

str=asctime(localtime(&date));
str[strlen(str)-1]='\000';
return str;
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*      Returne the version number and application      */
/*      name.                                           */
/*                                                      */
/********************************************************/
char *rou_getversion()

{
return VERSION"."RELEASE;
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*						        */
/*	Procedure to put a process in background        */
/*	mode.					        */
/*	Return the child process id.		        */
/*                                                      */
/********************************************************/
pid_t rou_divedivedive()

{
#define	OPEP	"subrouc:rou_divedivedive,"
pid_t childpid;

childpid=(pid_t)0;
switch (childpid=fork()) {
  case -1	:
    (void) fprintf(stderr,"%s, Unable to dive! (error=<%s>)",
			   OPEP,strerror(errno));
    break;
  case  0	:
    //	we are now in background mode
    (void) setsid();
    break;
  default	:
    //waiting for ballast to fill up :-}}
    (void) sleep(1);
    break;
  }
return childpid;
#undef	OPEP
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*	procedure to check if a process is	        */
/*	up and running.				        */
/*                                                      */
/********************************************************/
int rou_checkprocess(pid_t pidnum)

{
#define	SIGCHECK 0	//signal to check if process
			//is existing.
			//
int status;

status=false;
switch(pidnum) {
  case (pid_t)0	:	/*0 means no process	*/
    status=false;
    break;
  case (pid_t)1	:	/*init process always OK*/
    status=true;
    break;
  default	:	/*standard process	*/
    if (kill(pidnum,SIGCHECK)==0)
      status=true;
    break;
  }
return status;
#undef	SIGCHECK
}
/*
^L
*/
/********************************************************/
/*                                                      */
/*	Procedure to set/unset a lock 		        */
/*	return true if successful,		        */
/*	false otherwise.			        */
/*                                                      */
/********************************************************/
int rou_locking(const char *lockname,int lock,int tentative)

{
#define	OPEP	"subrou.c:lck_locking,"

int done;
char *fullname;
struct stat bufstat;
int phase;
int proceed;

done=false;
fullname=(char *)0;
phase=0;
proceed=true;
while (proceed==true) {
  switch (phase) {
    case 0	:	//setting lock filename
      if (lockname==(const char *)0) {
	(void) rou_alert(9,"%s lockname is missing (bug?)",OPEP);
	phase=999;	//big trouble, No need to go further
        }
      break;
    case 1	:	//setting lock filename
      char *name;

      name=(char *)calloc(sizeof(DIRLOCK)+strlen(lockname)+10,sizeof(char));
      (void) sprintf(name,"%s/%s.lock",DIRLOCK,lockname);
      fullname=rou_apppath(name);
      (void) free(name);
      break;
    case 2	:	//do we need to unlock ?
      if (lock==LCK_UNLOCK) {
	(void) rou_alert(9,"%s Request unlocking <%s>",OPEP,fullname);
	(void) unlink(fullname);
	done=true;
	phase=999;	//No need to go further
	}
      break;
    case 3	:	//checking if link already exist
      if (stat(fullname,&bufstat)<0) {
	phase++;	//link doesn't exist
	}
      break;
    case 4	:	//making lockname
      if (S_ISREG(bufstat.st_mode)!=0) {
        FILE *fichier;

        if ((fichier=fopen(fullname,"r"))!=(FILE *)0) {
          pid_t pid;
          char strloc[80];

	  (void) fgets(strloc,sizeof(strloc)-1,fichier);
	  (void) fclose(fichier);
          if (sscanf(strloc,"%lu",(u_long *)(&pid))==1) {
            (void) rou_alert(2,"Locking, check %d process active",pid);
	    if (rou_checkprocess(pid)==false) {
              (void) rou_alert(2,"Locking, remove unactive lock");
	      (void) unlink(fullname);
              }
	    else {
              (void) rou_alert(0,"lock check, found %d process still active",pid);
	      phase=999;	//no need to go further
	      }
	    }
	  }
	}
      break;
    case 5	:	//making lockname
      (void) rou_alert(6,"%s Request locking <%s>",OPEP,fullname);
      while (tentative>0) {
	int handle;

        tentative--;
        if ((handle=open(fullname,O_RDWR|O_EXCL|O_CREAT,0640))>=0) {
          char numid[30];

          (void) snprintf(numid,sizeof(numid),"%06d\n",getpid());
          (void) write(handle,numid,strlen(numid));
          (void) close(handle);
	  done=true;
	  break;	//breaking "tentative" loop
	  }
	else {
          (void) rou_alert(3,"Trying one more second to lock <%s> (error=<%s>)",
			      fullname,strerror(errno));
          (void) sleep(1);
	  }
	}
      break;
    default	:	//SAFE Guard
      if (done==false)
        (void) rou_alert(2,"Unable to set <%s> lock (config?)",lockname);
      proceed=false;
      break;
    }
  phase++;
  }
if (fullname!=(char *)0)
  (void) free(fullname);
return done;
#undef  LOCKNAM
#undef	OPEP
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
int rou_opensubrou()

{
int status;

status=0;
if (modopen==false) {
  debug=0;
  verbose=0;
  foreground=false;
  off64_time=(time_t)0;
  off_date=(time_t)0;
  if (rootdir!=(char *)0) {
    (void) free(rootdir);
    status=-1;          //Rootdir whould be found to be NULL
    }
  rootdir=(char *)0;
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
int rou_closesubrou()

{
int status;

status=0;
if (modopen==true) {
  if (rootdir!=(char *)0) {
    (void) free(rootdir);
    }
  modopen=false;
  }
return status;
}
