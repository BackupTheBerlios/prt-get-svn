# Makefile.am for prt-get by Johannes Winkelmann

bin_PROGRAMS=prt-get

prt_get_SOURCES= argparser.cpp argparser.h\
                 depresolver.cpp depresolver.h \
                 installtransaction.cpp installtransaction.h \
                 main.cpp \
                 package.cpp package.h \
                 pkgdb.cpp pkgdb.h \
                 prtget.cpp prtget.h \
                 repository.cpp repository.h \
                 stringhelper.cpp stringhelper.h \
                 process.cpp process.h \
                 configuration.cpp configuration.h \
                 signaldispatcher.cpp signaldispatcher.h \
                 lockfile.cpp lockfile.h \
                 file.cpp file.h \
                 locker.cpp locker.h

EXTRA_DIST = prt-get.conf

install-data-local: install-binPROGRAMS
	(cd $(DESTDIR)$(bindir); ln -sf prt-get prt-cache)
