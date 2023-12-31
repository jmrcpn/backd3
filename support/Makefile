#--------------------------------------------------------------------
#Executable generation area
#--------------------------------------------------------------------
clean   \
debug   \
prod	:

sauve	:	
	   @ echo	"Saving support component in RCS"
	   ./sauver $(SRC)

dotar   :
	  /bin/sh ./prepare $(RPMDIR) $(APPNAME) $(VERSION) $(RELEASE)
					
rpm     :
#--------------------------------------------------------------------
#Equivalences
#--------------------------------------------------------------------
SRC=	\
	tapedev tapelist \
	config.1  data.1  msdos.1  sources.1  user.1 \
	backd.spec \
	prepare  \
	install remove \
	Changes Makefile \
	sauver
#--------------------------------------------------------------------
APPNAME=backd
#--------------------------------------------------------------------
