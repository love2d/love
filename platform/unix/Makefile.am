ACLOCAL_AMFLAGS = -I platform/unix/m4
SUBDIRS = src
EXTRA_DIST = changes.txt license.txt readme.md \
	platform/unix/love.desktop.in
dist_man1_MANS = platform/unix/love.6

applicationsdir=$(datarootdir)/applications
mimeinfodir=$(datarootdir)/mime/packages
pixmapsdir=$(datarootdir)/pixmaps
mimeiconsdir=$(datarootdir)/icons/hicolor/scalable/mimetypes

applications_DATA = platform/unix/love.desktop
dist_mimeinfo_DATA = platform/unix/love.xml
dist_pixmaps_DATA = platform/unix/love.svg
dist_mimeicons_DATA = platform/unix/application-x-love-game.svg

platform/unix/love.desktop: platform/unix/love.desktop.in
	$(MKDIR_P) platform/unix
	rm -f $@ $@.tmp
	$(SED) \
		-e "s|@bindir[@]|$(bindir)|" \
		${srcdir}/$@.in > $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@
