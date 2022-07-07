GREEN = \033[0;32m
NC = \033[0m
CC = gcc
headers = -I../headers -I../headers/linmath.h -I../headers/stb -I../headers/stb/deprecated
lib_dir = ../lib
solibs = ../lib/libshader.so ../lib/libcamera.so ../lib/libmodel.so ../lib/liblight.so
glad_install_dir = /opt/glad
assimp_include_dir = /home/markbolding/Documents/assimp-5.0.1/include
assimp_config_dir = /home/markbolding/Documents/assimp-5.0.1/build/include
assimp_lib_dir = /home/markbolding/Documents/assimp-5.0.1/build/code

all: $(solibs)

$(lib_dir)/libglad.so: $(glad_install_dir)/src/glad.c
	mkdir -p ../lib
	$(CC) -g -c -fpic -I$(glad_install_dir)/include -o glad.o $<
	$(CC) -shared -o $@ glad.o

$(solibs): ../lib/lib%.so: %.c ../headers/%.h $(lib_dir)/libglad.so
	$(CC) -g -c -fpic $(headers) -DSHADER_DEBUG \
		-I$(assimp_include_dir) -DMODEL_DEBUG \
		-I$(assimp_config_dir) \
		-I$(glad_install_dir)/include -o $<.o $<
	$(CC) -shared -o $@ $<.o \
		-Wl,-rpath,$(assimp_lib_dir) -L$(assimp_lib_dir) \
		-L$(lib_dir) -lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm -f *.o
