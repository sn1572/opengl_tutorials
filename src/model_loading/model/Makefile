CC = g++
headers = ../../../headers
lib_dir = ../../../lib
libs = ../../../lib/libshader.so ../../../lib/libcamera.so ../../../lib/libmodel.so
lib_srcs = ../../shader.c ../../camera.c
binaries = main
glad_install_dir = /opt/glad
assimp_include_dir = /home/markbolding/Documents/assimp-5.0.1/include
assimp_config_dir = /home/markbolding/Documents/assimp-5.0.1/build/include
assimp_lib_dir = /home/markbolding/Documents/assimp-5.0.1/build/code

all: $(binaries)

$(libs): $(lib_srcs)
	cd ../../ && $(MAKE)

$(binaries): %: %.c $(libs)
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(assimp_include_dir) -I$(assimp_config_dir) \
		-I$(headers) -o $@.o $<
	$(CC) -o $@ $@.o -Wl,-rpath,$(lib_dir) -L$(lib_dir) \
		-Wl,-rpath,$(assimp_lib_dir) -L$(assimp_lib_dir) \
		-lshader -lglfw -lGL -lglad -ldl -lm -lassimp -lcamera -lmodel

.PHONY: clean

clean:
	rm *.o
	cd ../../ && $(MAKE) clean
