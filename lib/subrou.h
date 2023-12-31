// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/************************************************/
/*						*/
/*	Low level subroutine declaration	*/
/*						*/
/************************************************/
#ifndef	SUBROU
#define SUBROU

#include	<linux/types.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<stdio.h>
#include	<time.h>

#define	DATA	"data"	/*application data dir	*/
#define	CONF	"conf"	/*application config dir*/
#define	BACKUP	"bcklst"/*backup definition dir	*/
#define	SHELL	"shell"	/*shell directory	*/

#define LCK_UNLOCK   0	/*unlocking request	*/
#define	LCK_LOCK     1	/*locking requets	*/

/*defining boolean value			*/
typedef enum {false, true} bool;

//64 bit unsigned integer
typedef unsigned long long u_i64;

extern int	debug;	//application debug mode
extern int    verbose;	//verbose debug mode	
extern int foreground;	//app in foreground mode

extern char  *rootdir;  //application root directory

//to compute an application path with the root directory
extern char *rou_apppath(const char *path);

//local system time (plus offset if needed for debug purpose)
extern time_t rou_systime();

//to display message on console (verbose mode) or
//via syslog (LOG_DAEMON)
extern void rou_alert(const int dlevel,const char *fmt,...);

//To on purpose crash the application with an
//explication message
extern void rou_crash(const char *fmt,...);

//return the current system time in string format
extern char *rou_getstrfulldate(time_t curdate);

//return current system time in AAAAMMJJ format
extern u_long rou_normdate(time_t timestamp);

//return a time as date ONLY
extern char *rou_getstrdate(u_long curdate);

//return a time a hour:minute:second format
extern char *rou_getstrtime(time_t curdate);

//add month and days to a time (seen as a date)
extern u_long rou_caldate(time_t date,int month,int days);

//read an ascii file line, removeing line with comment
extern char *rou_getstr(FILE *fichier,char *str,int taille,int comment,char carcom);

//return the locale date (using time zone)
extern char *rou_localedate(time_t date);

//return program version
extern char *rou_getversion();

//procedure to put application in deamon mode
extern pid_t rou_divedivedive();

//routine to check if a proces is still up and running
extern int rou_checkprocess(pid_t pidnumber);

//lock application (to avoid running multiple daemon)
extern int rou_locking(const char *lockname,int lock,int tentative);

//homework done by module before starting to use it
int rou_opensubrou();

//homework dto be done by module before exiting
int rou_closesubrou();

#endif
