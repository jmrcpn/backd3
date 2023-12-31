// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/************************************************/
/*						*/
/*	management level, take care of all      */
/*      tap (or alike backup tap) access.       */
/*	to hanle an argv list and extract	*/
/*	parameters.				*/
/*						*/
/************************************************/
#ifndef	GESTAP
#define GESTAP

#include        "unipar.h"
#include        "unitap.h"

//write tape error status
typedef enum    {
        tap_ok,                 //everything fine
        tap_nodevice,           //device not available
        tap_wrongtapid,         //not expected tape ID
        tap_wronguuid,          //not expected tape UUID
        tap_nolock,             //unable to get lock on device
        tap_trouble             //unexpected trouble/bug
      }ETATYP;

//init a tap structure
extern TAPTYP *tap_newtap(char *label,uuid_t uuid,ARGTYP *params);

//procedure to write a new label on tape device
extern ETATYP tap_initheader(TAPTYP *tape);

//procedure to write a label on a tape device
extern ETATYP tap_writeheader(TAPTYP *tape);

//procedure to read a label on a tape device
extern TAPTYP *tap_readheader(char *device,u_i64 blksize);

//homework done by module before starting to use it
int tap_opengestap();

//homework to be done by module before exiting
int tap_closegestap();

#endif
