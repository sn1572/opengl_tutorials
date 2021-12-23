#ifndef MODEL_H
    #define MODEL_H

    #include <linmath.h>
    #include <stdio.h>
    #include <shader.h>
    #include <GLFW/glfw3.h>
    #include <stddef.h>

    typedef enum {
        MODEL_SUCCESS,
        MODEL_NO_MEM,
        MODEL_NULL_PTR,
        MODEL_GL_ERR,
    } model_error_t;

    struct Vertex {
        vec3 position;
        vec3 normal;
        vec3 texture_coordinates;
    };
    typedef struct Vertex Vertex;

    typedef enum {
        DIFFUSE,
        SPECULAR,
    } texture_t;
    
    struct Texture {
        unsigned int id;
        texture_t type;
    };
    typedef struct Texture Texture;

    struct Mesh {
        Vertex *       vertices;
        unsigned int   num_vertices;
        unsigned int * indices;
        unsigned int   num_indices;
        Texture *      textures;
        unsigned int   num_textures;
        unsigned int   VAO;
        unsigned int   VBO;
        unsigned int   EBO;
    };
    typedef struct Mesh Mesh;

    model_error_t setup_mesh(Mesh mesh);
    model_error_t draw(Shader * shader, Mesh mesh);

#endif
