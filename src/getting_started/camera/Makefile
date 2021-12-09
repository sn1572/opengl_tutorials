CC = gcc
headers = ../../../headers
libs = ../../../lib
binaries = camera 2camera 3camera exercise1
sources = camera.c 2camera.c 3camera.c exercise1.c
glad_install_dir = /opt/glad

all: $(binaries)

$(binaries): $(sources)
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(headers) -o $@.o $<
	$(CC) -o $@ $@.o -Wl,-rpath,$(libs) -L$(libs) -lshader -lcamera \
		-lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
