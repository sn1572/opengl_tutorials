CC = gcc
headers = -I../../../headers -I../../../headers/linmath.h -I../../../headers/stb -I../../../headers/stb/deprecated
lib_dir = ../../../lib
libs = $(lib_dir)lib/libshader.so $(lib_dir)lib/libcamera.so $(lib_dir)lib/libmodel.so $(lib_dir)/liblight.so
lib_srcs = ../../shader.c ../../camera.c ../../model.c ../../light.c
binaries = main
glad_install_dir = ${GLAD_DIR}
assimp_include_dir = ${ASSIMP_DIR}/include
assimp_config_dir = ${ASSIMP_DIR}/include
assimp_lib_dir = ${ASSIMP_DIR}/lib

all: $(binaries)

$(libs): $(lib_srcs)
	cd ../../ && $(MAKE)

$(binaries): %: %.c $(libs)
	$(CC) -g -c -I$(glad_install_dir)/include \
		-I$(assimp_include_dir) -I$(assimp_config_dir) \
		$(headers) -o $@.o $<

	$(CC) -o $@ $@.o -Wl,-rpath,$(lib_dir) -L$(lib_dir) \
		-L${ASSIMP_DIR}/lib -Wl,-rpath,$(assimp_lib_dir) -L$(assimp_lib_dir) \
		-lshader -lglfw -lGL -lglad -ldl -lm -lassimp -lcamera -lmodel \
		-llight

.PHONY: clean

clean:
	rm -f *.o
	cd ../../ && $(MAKE) clean
