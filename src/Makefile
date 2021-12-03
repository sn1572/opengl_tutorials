GREEN = \033[0;32m
NC = \033[0m
CC = gcc
solibs = libshader.so libcamera.so
headers = ../headers
libs = ../lib
glad_install_dir = /home/mark/Documents/C/glad

all: $(solibs)

$(libs)/glad.o: $(glad_install_dir)/src/glad.c
	$(CC) -g -c -I$(glad_install_dir)/include -o $@ $< \
	&& mv 

$(solibs): lib%.so: %.c glad.o
	$(CC) -g -c -fpic -I$(headers) \
		-I$(glad_install_dir)/include -o $<.o $<
	$(CC) -shared -o $@ $<.o -Wl,-rpath,$(libs) \
		-lglfw -lGL -lglad -ldl -lm \
		&& mv $@ ../lib

.PHONY: clean

clean:
	rm *.o
