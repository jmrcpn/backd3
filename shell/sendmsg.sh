#! /bin/sh
#--------------------------------------------------------------------------
# $1= Messages TYPE
#	S_OK		backup successful
#	S_NOTAPE	No tape available for backup
#	S_MISSING	No tape in tape reader.
#	
# $2= TAPE ID.
# $3= Backed scheduled starting time
#--------------------------------------------------------------------------
#setting local variables
PRINTER=lethp5l
LOGS=/tmp/bcklogs.$$
#email originator
ORIGMNG=backd@`dnsdomainname`
#--------------------------------------------------------------------------
cd `dirname $0`
if [ -f /etc/sysconfig/backd ]; then
 . /etc/sysconfig/backd
fi
#--------------------------------------------------------------------------
#storing backlog somewhere in /tmp
cat - > $LOGS
#--------------------------------------------------------------------------
#to check if BCKMNG is already defined
if [ ! $BCKMNG ] ; then
  export BCKMNG=root
fi
FROM="Your own Backup daemon <backd>";
TO="$BCKMNG"
#--------------------------------------------------------------------------
#Shell to send mail to back managers.
#--------------------------------------------------------------------------
case "$1" in
#--------------------------------------------------------------------------
#Message to confirm everything ok
#--------------------------------------------------------------------------
  "S_OK" )
      SUBJECT="Backup successful on tape $2" 
#     printing logs
#	Letterdj is defined in/etc/a2ps.cfg
      cat $LOGS | 							\
	      /usr/bin/a2ps						\
	     		-1						\
		       	--medium=Letterdj				\
		       	-P$PRINTER					\
			--margin=30					\
			--center-title=$2				\
			--left-footer					\
			--right-footer					\
			--pro=color					\
			--font-size=9					\
			--landscape					\
			--stdin=' '					\
			-b						\
			-q						\
			> /dev/null
    ;;
#--------------------------------------------------------------------------
#Message to signal no tape available in the list
#--------------------------------------------------------------------------
  "S_NOTAPE" )
      SUBJECT="No tape available to do backup" 
      MSG="Backup scheduled at $3"
    ;;
#--------------------------------------------------------------------------
#Message to request Tape in tape reader
#--------------------------------------------------------------------------
  "S_MISSING" )
      SUBJECT="Please Insert tape $2 in tape reader" 
      MSG="Backup scheduled at $3"
    ;;
#--------------------------------------------------------------------------
#Tape is in tape reader and ready
#--------------------------------------------------------------------------
  "S_TAPEIN" )
      SUBJECT="Tape $2 now in tape reader" 
      MSG="Please keep tape $2 in the tape reader, backup scheduled at $3"
    ;;
#--------------------------------------------------------------------------
#Tape in tape reader is not the right one
#--------------------------------------------------------------------------
  "S_WRONGTAPE" )
      SUBJECT="$2 tape is not in tape-reader" 
      MSG="You have the wrong tape in the tape reader"
    ;;
#--------------------------------------------------------------------------
#unknow request
#--------------------------------------------------------------------------
  "S_UNKNOWN" | \
  * )
      SUBJECT="Unknow error with tape $2" 
      MSG="There is something really bad going on with backup process"
    ;;
  esac
#--------------------------------------------------------------------------
#sending the actual message.
(
echo "From: "$FROM
echo "To: "$TO
echo "Subject: "$SUBJECT
echo
echo $MSG
cat $LOGS
echo 
) | s-nail -r $ORIGMNG -s"$SUBJECT" $BCKMNG ;
#--------------------------------------------------------------------------
#removing logs info
rm $LOGS
#--------------------------------------------------------------------------
