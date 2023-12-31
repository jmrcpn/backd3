// vim: smarttab tabstop=8 shiftwidth=2 expandtab
/********************************************************/
/*							*/
/*	Low level subroutine declaration	        */
/*	handle tape contents                            */
/*							*/
/********************************************************/
#ifndef	UNITAP
#define UNITAP

#include        <time.h>
#include        <uuid/uuid.h>

#define LABSIZE 30              //label maximun size

//Tape header structure
typedef struct  {
        char    id[3][LABSIZE]; //tape ID (current, previous, next)
        int     numrot;         //tape number of rotation (aging)
        time_t  stamp;          //backup stamp date
        time_t  tobkept;        //Backup retention date
        char    vers[10];       //backd version used to do backup
        char    uuid[40];       //tape marker uniqid
        char    *device;        //device used by tape
        u_i64   blksize;        //tape blocksize (8 bytes)
        u_i64   lastblk;        //last position on the tape
        u_i64   block;          //tape size in Mbytes
        }TAPTYP;

//free memory used by a tap structure
extern TAPTYP *tap_freetape(TAPTYP *data);

//convert a tap structure to a header
void *tap_tapetoheader(TAPTYP *data);

//convert a tap header string to a tape structure
extern TAPTYP *tap_headertotape(void *header);

//homework done by module before starting to use it
int tap_openunitap();

//homework dto be done by module before exiting
int tap_closeunitap();

#endif
