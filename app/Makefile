#--------------------------------------------------------------------
#Executable generation area
#--------------------------------------------------------------------
debug	: 
	   @ $(MAKE) OPTIME="-g" DEBUG="-DDEBUG" exe 
	   @ echo "application compiled in '$@' mode now ready"

prod	:
	   @ $(MAKE) OPTIME="-g2" exe
	   @ echo "application compiled in '$@' mode now ready"

exe	:
	   @ $(MAKE) $(EXE)


clean	:
	   - rm -fr *.o $(EXE)
	   - rm -fr ../bin/*

#--------------------------------------------------------------------
#Equivalences
#--------------------------------------------------------------------
EXE=								\
	reader marker backd					\


SRC=								\
	backd.c							\
	Makefile

#--------------------------------------------------------------------
#definitions
#--------------------------------------------------------------------
CC	=  gcc
LD	=  gcc
CFLAGS	=  -I ../lib -Wall $(OPTIME)
LIBBCK	=  ../lib/libbck.a
LIBS	=  $(LIBBCK) -luuid
#--------------------------------------------------------------------
#Dependances
#--------------------------------------------------------------------
backd2	: toremake backd.o
	  @ mkdir -p ../bin
	  @ $(LD) $(LDFLAGS) -o ../bin/backd2 backd.o $(LIBS)

backd								\
marker								\
reader								\
	: backd2
	  @ ln -nsf ./backd2 ../bin/backd-$@

backd.o:  backd.c						\
	   ../lib/gestap.h					\
	   ../lib/subrou.h					\
	   $(LIBBCK)

toremake:  Makefile $(LIBBCK)
	   touch toremake
	   - rm -f $(EXE) *.o

#--------------------------------------------------------------------
