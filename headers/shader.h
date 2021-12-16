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
        SHADER_FS_ERR,
    } shader_err_t;

    #define err_print(msg){\
        fprintf(stderr, "%s %d: "msg, __FILE__, __LINE__);\
    }

    struct Shader{
        unsigned int ID;
        struct Shader * self;
        shader_err_t (*load)(struct Shader * self, char * vertexPath,
                             char * fragmentPath);
        shader_err_t (*use)(struct Shader * self);
        shader_err_t (*setBool)(struct Shader * self, const char * name,
                                int value);
        shader_err_t (*setInt)(struct Shader * self, const char * name,
                               int value);
        shader_err_t (*setFloat)(struct Shader * self, const char * name,
                                 float value);
        shader_err_t (*setVec3)(struct Shader * self, const char * name, 
                                float x, float y, float z);
    };

    shader_err_t readFile(const char * fname, char ** buffer);
    shader_err_t load(struct Shader * self, char * vertexPath,
                      char * fragmentPath);
    shader_err_t use(struct Shader * self);
    shader_err_t setBool(struct Shader * self, const char * name, int value);
    shader_err_t setInt(struct Shader * self, const char * name, int value);
    shader_err_t setFloat(struct Shader * self, const char * name, float value);
    shader_err_t setVec3(struct Shader * self, const char * name, float x, float y,
                 float z);
    struct Shader * shaderInit();
    shader_err_t checkCompileErrors(unsigned int shader, char * type);
    void shader_introspection(struct Shader * shaders);
#endif
