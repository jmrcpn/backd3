#--------------------------------------------------------------------
#Executable generation area
#--------------------------------------------------------------------
debug	:  
	   @ $(MAKE) OPTIME="-g" objs
	   @ echo "library compiled in '$@' mode, now ready"

prod	:  
	   @ $(MAKE) OPTIME="-O3" objs
	   @ echo "library compiled in '$@' mode, now ready"

sauve	:	
	   @ echo	"Saving source component in RCS"
	   ../support/sauver $(SRC)

clean	:
	   - rm -fr *.o *.a

#--------------------------------------------------------------------
#Equivalences
#--------------------------------------------------------------------
OBJS=						\
	gestap.o				\
	unitap.o unipar.o			\
	subrou.o 

SRC=						\
	gestap.h gestap.c			\
	unipar.h unipar.c			\
	unitap.h unitap.c			\
	subrou.h subrou.c			\
	Makefile

objs	: 
	  @ $(MAKE) $(LIBBCK)

libbck.a					\
	: toremake $(OBJS)
	  @ ar -cr $(LIBBCK) $(OBJS)

#--------------------------------------------------------------------
#Dependances
#--------------------------------------------------------------------

gestap.o:  gestap.c gestap.h				\
	   subrou.h

unitap.o:  unitap.c unitap.h

unipar.o:  unipar.c unipar.h

subrou.o:  subrou.c subrou.h


gestap.h:  unipar.h unitap.h
	   touch $@

unitap.h					\
unipar.h					\
	:  subrou.h
	   touch $@

toremake:  Makefile
	   touch toremake
	   - rm -f $(OBJS) $(LIBBCK)
#--------------------------------------------------------------------
#definitions
#--------------------------------------------------------------------
CC=gcc
LD=gcc
CFLAGS= -Wall $(OPTIME)
LIBBCK=libbck.a
#--------------------------------------------------------------------
