#ifndef SHADER_H
	#define SHADER_H

	#include <glad/glad.h>
	#include <stdio.h>
	#include <stdlib.h>

    typedef enum {
        SHADER_NO_ERR,
        SHADER_NULL_PTR,
        SHADER_GL_ERR,
        SHADER_NO_MEM,
    } shader_err_t;

    #define err_print(msg){\
        fprintf(stderr, "%s %d: "msg, __FILE__, __LINE__);\
    }

	struct Shader{
		unsigned int ID;
		struct Shader * self;
		shader_err_t (*load)(struct Shader * self, char * vertexPath,
                             char * fragmentPath);
		void (*use)(struct Shader * self);
		void (*setBool)(struct Shader * self, const char * name, int value);
		void (*setInt)(struct Shader * self, const char * name, int value);
		void (*setFloat)(struct Shader * self, const char * name, float value);
		void (*setVec3)(struct Shader * self, const char * name,
				 float x, float y, float z);
	};

	void readFile(const char * fname, char ** buffer);
	shader_err_t load(struct Shader * self, char * vertexPath,
                      char * fragmentPath);
	void use(struct Shader * self);
	void setBool(struct Shader * self, const char * name, int value);
	void setInt(struct Shader * self, const char * name, int value);
	void setFloat(struct Shader * self, const char * name, float value);
	void setVec3(struct Shader * self, const char * name, float x, float y,
                 float z);
	struct Shader * shaderInit();
	shader_err_t checkCompileErrors(unsigned int shader, char * type);
	void shader_introspection(struct Shader * shaders);
#endif
