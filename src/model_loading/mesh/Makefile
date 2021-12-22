CC = gcc
headers = ../../../headers
lib_dir = ../../../lib
libs = ../../../lib/shader ../../../lib/camera
lib_srcs = ../../shader.c ../../camera.c
binaries = main
glad_install_dir = /home/mark/Documents/C/glad
assimp_include_dir = /home/mark/Projects/assimp-5.0.1/build/include/assimp
assimp_lib_dir = /home/mark/Projects/assimp-5.0.1/build/code

all: $(binaries)

$(libs): $(lib_srcs)
	cd ../../ && $(MAKE)

$(binaries): %: %.c $(libs)
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(assimp_include_dir) -I$(headers) -o $@.o $<
	$(CC) -o $@ $@.o -Wl,-rpath,$(lib_dir) -L$(lib_dir) \
		-Wl,-rpath,$(assimp_lib_dir) -L$(assimp_lib_dir) \
		-lshader -lcamera -lglfw -lGL -lglad -ldl -lm -lassimp

.PHONY: clean

clean:
	rm *.o
	cd ../../ && $(MAKE) clean
