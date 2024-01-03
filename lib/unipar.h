// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Low level subroutine declaration	        */
/*	to hanle an argv list and extract	        */
/*	parameters.				        */
/*							*/
/********************************************************/
#ifndef	UNIPAR
#define UNIPAR

#include        <uuid/uuid.h>

#include        "subrou.h"

//list of back program
typedef enum {			//application type
	wha_backd,		//'backd', the application main daemon
	wha_marker,		//'backd-marker' to mark the TAPE
	wha_reader,		//'backd-reader' To read TAPE header
	wha_recover,		//'backd-recover' To recover all or one
				//backup component
	wha_unknown		//default value (trouble)
	}WHATTYPE;

//structure of argument
typedef struct	{
	char *device;		//The (tape) device 
        char *pool;             //the tape used pool
        uuid_t uuid;            //session unique ID
        u_i64   blksize;        //tape blocksize (8 bytes)
	int argc;		//number of main argument
	char **argv;	        //list of argument
	}ARGTYP;

//free allocated memory used by a ARGTYP structure
extern ARGTYP *par_freeparams(ARGTYP *params);

//allocated memory and parse an argment list
extern ARGTYP *par_getparams(int argc,char * const argv[],const char *optstring);

//find out about the program name
extern WHATTYPE par_whatami(char *backdname);

//display available parameters according backd called name
extern int par_usage(char *name,WHATTYPE called);

//homework done by module before starting to use it
int par_openunipar();

//homework dto be done by module before exiting
int par_closeunipar();

#endif
