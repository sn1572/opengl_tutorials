GREEN = \033[0;32m
NC = \033[0m
CC = g++
headers = ../headers
libs = ../lib
solibs = ../lib/libshader.so ../lib/libcamera.so ../lib/libmodel.so
glad_install_dir = /opt/glad
assimp_include_dir = /home/markbolding/Documents/assimp-5.0.1/include
assimp_config_dir = /home/markbolding/Documents/assimp-5.0.1/build/include
assimp_lib_dir = /home/markbolding/Documents/assimp-5.0.1/build/code

all: $(solibs)

$(libs)/libglad.so: $(glad_install_dir)/src/glad.c
	mkdir -p ../lib
	$(CC) -g -c -fpic -I$(glad_install_dir)/include -o glad.o $<
	$(CC) -shared -o $@ glad.o

$(solibs): ../lib/lib%.so: %.c ../headers/%.h $(libs)/libglad.so
	$(CC) -g -c -fpic -I$(headers) -DSHADER_DEBUG \
		-I$(assimp_include_dir) -DMODEL_DEBUG \
		-I$(assimp_config_dir) \
		-I$(glad_install_dir)/include -o $<.o $<
	$(CC) -shared -o $@ $<.o \
		-Wl,-rpath,$(assimp_lib_dir) -L$(assimp_lib_dir) \
		-L$(libs) -lglfw -lGL -lglad -ldl -lm

.PHONY: clean

clean:
	rm *.o
