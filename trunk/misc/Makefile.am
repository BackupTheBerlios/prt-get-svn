EXTRA_DIST = prt-get_complete prt-get.aliases prt-get.deplist

install-data-local:
	$(INSTALL) -D -m 644 prt-get.deplist \
		$(DESTDIR)/var/lib/pkg/prt-get.deplist
	$(INSTALL) -D -m 644 prt-get.aliases \
		$(DESTDIR)/var/lib/pkg/prt-get.aliases
