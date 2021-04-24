bin_SCRIPTS = 
dist_doc_DATA = README.md
SUBDIRS = src

GREEN = \033[0;32m
NC = \033[0m

solibs = libshader.so libcamera.so

.PHONY: clean clean-local

all-am: $(solibs)

$(solibs): lib%.so: %.c
	gcc -g -c -fpic -o $<.o $<
	gcc -shared -o $@ $<.o -Wl,-rpath,$(LD_LIBRARY_PATH) \
		-lshader -lglfw -lGL -lglad -ldl -lm

clean:
	rm *.o

clean-local:
	for dir in autom4te.cache build dist bin; do \
		test -d $$dir && rm -r $$dir; \
	done; \
	for file in config.status configure config.log aclocal.m4 \
		install-sh missing compile samples \
		Makefile.in INSTALL COPYING Makefile; do \
		test -f $$file && rm $$file; \
	done;
