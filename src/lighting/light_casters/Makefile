CC = gcc
headers = ../../../headers
lib_dir = ../../../lib
libs = ../../../lib/shader ../../../lib/camera
lib_srcs = ../../shader.c ../../camera.c
binaries = directional spotlight point
glad_install_dir = /opt/glad

all: $(binaries)

$(libs): $(lib_srcs)
	cd ../../ && $(MAKE)

$(binaries): %: %.c $(libs)
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(headers) -o $@.o $<
	$(CC) -o $@ $@.o -Wl,-rpath,$(lib_dir) -L$(lib_dir) -lshader -lcamera \
		-lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
	cd ../../ && $(MAKE) clean
