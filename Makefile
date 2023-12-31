#--------------------------------------------------------------------
include	Makefile.git
#--------------------------------------------------------------------
#Executable generation area
#--------------------------------------------------------------------
prod								\
debug								\
	:  
	   @ for i in $(SUBDIR) ;				\
	       do						\
	       $(MAKE) -s -C $$i $@ ;				\
	       done

clean	:
	   @ for i in $(SUBDIR) ;				\
	       do						\
	       $(MAKE) -s -C $$i $@ ;				\
	       done
	   @ - rm -fr $(APPNAME)-*
	   @ - rm -fr dotest $(TESTDIR)

#--------------------------------------------------------------------
#entry to test/simulate application
#--------------------------------------------------------------------
#test parameters
PARTEST	=							\
	   -d 0							\
	   -r $(TESTDIR)					\
	   TST-271828						\
	   -u /dev/tap1

TSTPOOL	=							\
	   TST-355113						\
	   TST-314159

#to test tape marker
marker	:  debug dotest
	   @ echo "starting backd-marker test"
	   @ ./bin/backd-$@ $(PARTEST)
	  
#to test tape marker with valgrind 
vlmarker:  debug dotest
	   @ echo "backd-marker valgrind test"
	   @ valgrind 						\
		-s						\
		--leak-check=full				\
		./bin/backd-marker $(PARTEST)
	   
#to debug tape marker
gdmarker: debug dotest
	  @ gdb --args ./bin/backd-marker $(PARTEST)

#--------------------------------------------------------------------
#building testarea
#--------------------------------------------------------------------
#refresh test area from scratch
newtest :  notest dotest

#refresh test atrea from scratch
newtest :  notest dotest

#to create a test area
dotest	:
	   @ mkdir -p $(TESTDIR)/{dev,conf,var/run/backd}
	   @ dd							\
		status=none					\
		if=/dev/zero					\
		of=$(TESTDIR)/dev/tap1				\
	       	bs=1M						\
	       	count=$(DEVSIZE)
	   @ cp $(TESTDIR)/dev/tap1 $(TESTDIR)/dev/tap2
	   @ touch $@

#destrpying the testarea
notest	:
	   @ rm -fr $(TESTDIR)
	   @ rm dotest

#pseudo tape size in MegaBytes
DEVSIZE	=  500
TESTDIR	=  ./test-area
#====================================================================

#--------------------------------------------------------------------
#Installation procedure
#--------------------------------------------------------------------
install	:
	   #preparing /var/lib/backd directory
	   @ install -d $(DESTDIR)/var/lib/$(APPNAME)
	   @ install *build_date $(DESTDIR)/var/lib/$(APPNAME)
	   @ cp -a bin $(DESTDIR)/var/lib/$(APPNAME)
	   @ install -d $(DESTDIR)/var/lib/$(APPNAME)/conf
	   @ install conf/{standard.sch,tapedev,tapelist,bckdlist.exp} \
		     $(DESTDIR)/var/lib/$(APPNAME)/conf
	   @ install -d $(DESTDIR)/var/lib/$(APPNAME)/shell
	   @ install shell/sendmsg.sh $(DESTDIR)/var/lib/$(APPNAME)/shell
	   @ install -d $(DESTDIR)/etc/rc.d/init.d
	   @ install shell/backd.sh $(DESTDIR)/etc/rc.d/init.d/backd
	   @ install -d $(DESTDIR)/etc/sysconfig
	   @ install conf/backd.conf $(DESTDIR)/etc/sysconfig/backd
#--------------------------------------------------------------------
#Equivalences
#--------------------------------------------------------------------
SUBDIR	= 								\
	   lib								\
	   app								\
	   conf								\
	   man								\
	   shell							\
	   support 

#--------------------------------------------------------------------
#definitions
#--------------------------------------------------------------------
APPNAME	=  backd3
#--------------------------------------------------------------------
