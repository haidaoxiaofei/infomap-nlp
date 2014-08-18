include Makefile.filenames

#dist_noinst_DATA = filenames.h

##bin_SCRIPTS = infomap-build infomap-install

##infomap-build : infomap-build.in
##	sed -e 's,__PKG_DATA_DIR__,@pkgdatadir@,' infomap-build.in >infomap-build

##infomap-install : infomap-install.in

# hardcoded mkdir_p = should be automatically generated; we think the macro should be AM_PROG_MKDIR_P
# but we don't have that; if you have trouble with this it may be because your system doesn't understand 
# mkdir -p
mkdir_p = mkdir -p

bin_SCRIPTS = infomap-install infomap-build

install-exec-local :
	mv infomap-install infomap-install.old
	sed -e "s,^prefix=,prefix=$(prefix)," \
           infomap-install.old >infomap-install
	$(INSTALL) infomap-install $(bindir)/infomap-install
	mv infomap-build infomap-build.old
	sed -e "s,^prefix=,prefix=$(prefix)," \
	    -e "s,^exec_prefix=,exec_prefix=$(exec_prefix)," \
	   infomap-build.old >infomap-build
	$(INSTALL) infomap-build $(bindir)/infomap-build

install-exec-hook :
	mv -f infomap-install.old infomap-install
	mv -f infomap-build.old infomap-build

install-data-local :
	mv Makefile.data Makefile.data.old
	sed -e "s,^prefix=,prefix=$(prefix)," \
	   Makefile.data.old >Makefile.data
	$(mkdir_p) $(datadir)/$(PACKAGE)
	$(INSTALL) Makefile.data $(datadir)/$(PACKAGE)/Makefile.data
	mv default-params default-params.old
	sed -e "s,^prefix=,prefix=$(prefix)," \
	   default-params.old >default-params
	$(mkdir_p) $(datadir)/$(PACKAGE)
	$(INSTALL) default-params $(datadir)/$(PACKAGE)/Makefile.data

install-data-hook :
	mv -f Makefile.data.old Makefile.data
	mv -f default-params.old default-params





pkgdata_DATA = Makefile.data Makefile.filenames install.control \
	       default-params stop.list valid_chars.en

# Maybe we'll do this later, but I don't know how to deal with
# DBM .dir/.pag file pairs reasonably.
# So for now we just distribute install.control directly,
# with actual filenames rather than header/Makefile variables.
#install.control : install.control.in filenames.h 
#	cpp -include filenames.h install.control.in | \
#	   sed -e 's/^"\([^"]*\)" *:/\1:/' >install.control


EXTRA_DIST = Makefile.data.in Makefile.filenames install.control \
             default-params.in stop.list filenames.h valid_chars.en

CLEANFILES = filenames.h


