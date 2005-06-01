EXTRA_DIST = prt-get_complete prt-get.aliases prt-get.deplist prt-get.conf

install-data-local:
	$(INSTALL) -D -m 644 $(srcdir)/prt-get.deplist \
		$(DESTDIR)$(localstatedir)/lib/pkg/prt-get.deplist
	$(INSTALL) -D -m 644 $(srcdir)/prt-get.aliases \
		$(DESTDIR)$(localstatedir)/lib/pkg/prt-get.aliases
	$(INSTALL) -D -m 644 $(srcdir)/prt-get.conf \
		$(DESTDIR)$(localstatedir)/etc/prt-get.conf

uninstall-local:
	rm $(DESTDIR)$(localstatedir)/lib/pkg/prt-get.deplist
	rm $(DESTDIR)$(localstatedir)/lib/pkg/prt-get.aliases
	rm $(DESTDIR)$(localstatedir)/etc/prt-get.conf
