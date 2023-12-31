#--------------------------------------------------------------------
include	Makefile.git
#--------------------------------------------------------------------
#Executable generation area
#--------------------------------------------------------------------
prod	\
debug	:  
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
	   conf								\
	   lib								\
	   app								\
	   man								\
	   shell							\
	   support 

#--------------------------------------------------------------------
#definitions
#--------------------------------------------------------------------
APPNAME	=	backd3
#--------------------------------------------------------------------
