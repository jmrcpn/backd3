#--------------------------------------------------------------------
#default make
build	:  clean dovers prod
#--------------------------------------------------------------------
#Makefile with all data for GIT
include	Makefile.git
#Makefile with all data about RPM
include	Makefile.pkg
#Makefile to build and test application
include	Makefile.bld
#--------------------------------------------------------------------
