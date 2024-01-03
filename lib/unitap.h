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

#define LABESIZ 30              //label maximun size
#define POOLSIZ 15              //pool name maximun size

//Tape header structure
typedef struct  {
        char    id[3][LABESIZ]; //tape ID (current, previous, next)
        char    pool[POOLSIZ];  //tape pool id
        int     numrot;         //tape number of rotation (aging)
        time_t  stamp;          //backup stamp date
        time_t  tobkept;        //Backup retention date
        char    vers[10];       //backd version used to do backup
        char    uuid[40];       //tape marker uniqid
        char    device[60];     //device used by tape
        u_i64   blksize;        //tape blocksize (8 bytes)
        u_i64   lastblk;        //last position on the tape
        ulong   lastused;       //tape lastuse
        u_int   frozen;         //how long the tape must kept
        u_int   cycled;         //number of time tape was cycled
        u_int   pidlock;        //Tape is lock by backd daemon
        }TAPTYP;

//free memory used by a tap structure
extern TAPTYP *tap_freetape(TAPTYP *data);

//prepare a tape structure
extern TAPTYP *tap_newtape();

//convert a tap structure to a string
extern char *tap_tapetostr(TAPTYP *tape);

//convert a string to a tape structure
extern TAPTYP *tap_strtotape(const char *data);

//convert a tap header string to a tape structure
extern TAPTYP *tap_headertotape(void *header);

//convert a tap structure to a header
void *tap_tapetoheader(TAPTYP *data);

//homework done by module before starting to use it
int tap_openunitap();

//homework dto be done by module before exiting
int tap_closeunitap();

#endif
