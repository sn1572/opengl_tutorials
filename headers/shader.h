#ifndef SHADER_H
    #define SHADER_H

    #include <glad/glad.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <linmath.h>

    typedef enum {
        SHADER_NO_ERR   =  0,
        SHADER_NULL_PTR = -1,
        SHADER_GL_ERR   = -2,
        SHADER_NO_MEM   = -3,
        SHADER_FS_ERR   = -4,
    } shader_err_t;

    #ifndef err_print
        #define err_print(msg){\
            fprintf(stderr, "%s %d: "msg"\n", __FILE__, __LINE__);\
        }
    #endif

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
                                vec3 vec);
    };
    typedef struct Shader Shader;

    shader_err_t readFile(const char * fname, char ** buffer);
    shader_err_t load(struct Shader * self, char * vertexPath,
                      char * fragmentPath);
    shader_err_t shaderLoad(struct Shader * self, char * vertexPath,
                            char * fragmentPath, char * geomPath);
    shader_err_t use(struct Shader * self);
    shader_err_t setBool(struct Shader * self, const char * name, int value);
    shader_err_t setInt(struct Shader * self, const char * name, int value);
    shader_err_t setFloat(struct Shader * self, const char * name, float value);
    shader_err_t setVec3(struct Shader * self, const char * name, vec3 vec);
    shader_err_t flatten(float * out, mat4x4 M);
    shader_err_t setMat4x4(struct Shader * self, const char * name,
                           mat4x4 matrix);
    struct Shader * shaderInit();
    shader_err_t checkCompileErrors(unsigned int shader, char * type);
    void shader_introspection(struct Shader * shaders);
#endif
