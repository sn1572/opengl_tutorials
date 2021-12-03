GREEN = \033[0;32m
NC = \033[0m
CC = gcc
headers = ../headers
libs = ../lib
solibs = libshader.so libcamera.so
glad_install_dir = /home/mark/Documents/C/glad

all: $(solibs)

$(libs)/glad.so: $(glad_install_dir)/src/glad.c
	$(CC) -g -c -fpic -I$(glad_install_dir)/include -o glad.o $<
	$(CC) -shared -o $@ glad.o

$(solibs): lib%.so: %.c $(libs)/glad.so
	$(CC) -g -c -fpic -I$(headers) \
		-I$(glad_install_dir)/include -o $<.o $<
	$(CC) -shared -o $@ $<.o -Wl,-rpath,$(libs) \
		-L$(libs) -lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
