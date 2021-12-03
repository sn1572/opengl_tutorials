GREEN = \033[0;32m
NC = \033[0m
solibs = libshader.so libcamera.so
headers = ../headers
glad_install_dir = /home/mark/Documents/C/glad

all: $(solibs)

$(solibs): lib%.so: %.c
	gcc -g -c -fpic -I$(headers) \
		-I$(glad_install_dir) -o $<.o $<
	gcc -shared -o $@ $<.o -Wl,-rpath,$(LD_LIBRARY_PATH) \
		-lshader -lglfw -lGL -lglad -ldl -lm \
		&& mv $@ ../lib

.PHONY: clean

clean:
	rm *.o
