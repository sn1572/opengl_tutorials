GREEN = \033[0;32m
NC = \033[0m
CC = gcc
headers = ../headers
libs = ../lib
solibs = ../lib/libshader.so ../lib/libcamera.so
glad_install_dir = /opt/glad

all: $(solibs)

$(libs)/libglad.so: $(glad_install_dir)/src/glad.c
	mkdir -p ../lib
	$(CC) -g -c -fpic -I$(glad_install_dir)/include -o glad.o $<
	$(CC) -shared -o $@ glad.o

$(solibs): ../lib/lib%.so: %.c ../headers/%.h $(libs)/libglad.so
	$(CC) -g -c -fpic -I$(headers) \
		-I$(glad_install_dir)/include -o $<.o $<
	$(CC) -shared -o $@ $<.o \
		-L$(libs) -lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
