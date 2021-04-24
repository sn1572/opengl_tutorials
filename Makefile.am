dist_doc_DATA = README.md
SUBDIRS = src

.PHONY: clean-local

clean-local:
	for dir in autom4te.cache build dist bin; do \
		test -d $$dir && rm -r $$dir; \
	done; \
	for file in config.status configure config.log aclocal.m4 \
		install-sh missing compile samples \
		config.guess config.log config.status config.sub \
		libtool ltmain.sh \
		Makefile.in INSTALL COPYING Makefile; do \
		test -f $$file && rm $$file; \
	done;
