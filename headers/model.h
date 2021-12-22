#ifndef MODEL_H
    #define MODEL_H

    #include <linmath.h>
    #include <stdio.h>
    #include <shader.h>
    #include <GLFW/glfw3.h>
    #include <stddef.h>

    typedef enum {
        MODEL_SUCCESS;
        MODEL_NO_MEM;
        MODEL_NULL_PTR;
        MODEL_LINMATH;
    } model_error_t;

    struct Vertex {
        vec3 position;
        vec3 normal;
        vec3 texture_coordinates;
    };

    typedef enum {
        DIFFUSE_TEXTURE,
        SPECULAR_TEXTURE,
    } texture_t;
    
    struct Texture {
        unsigned int id;
        texture_t type;
    };

    struct Mesh {
        Vertex *       vertices;
        unsigned int * indices;
        Texture *      textures;
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };

    model_error_t setup_mesh(Mesh mesh);
    model_error_t draw(Shader * shader, Mesh mesh);

#endif
