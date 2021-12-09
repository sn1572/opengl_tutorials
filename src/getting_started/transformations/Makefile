CC = gcc
headers = ../../../headers
libs = ../../../lib
binaries = transformations 2transformations exercise1 exercise2
glad_install_dir = /opt/glad

all: $(binaries)

$(binaries): %: %.c
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(headers) -o $@.o $<
	$(CC) -o $@ $@.o -Wl,-rpath,$(libs) -L$(libs) -lshader -lcamera \
		-lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
