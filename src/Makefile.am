GREEN = \033[0;32m
NC = \033[0m

solibs = libshader.so libcamera.so

.PHONY: clean

all: $(solibs)

$(solibs): lib%.so: %.c
	gcc -g -c -fpic -o $<.o $<
	gcc -shared -o $@ $<.o -Wl,-rpath,$(LD_LIBRARY_PATH) \
		-lshader -lglfw -lGL -lglad -ldl -lm

clean:
	rm *.o
