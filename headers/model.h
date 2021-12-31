#ifndef MODEL_H
    #define MODEL_H

    #include <linmath.h>
    #include <stdio.h>
    #include <shader.h>
    #include <GLFW/glfw3.h>
    #include <stddef.h>
    #include <assimp/cimport.h>
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    #include <assimp/material.h>
    #include <libgen.h>
    #define STB_IMAGE_IMPLEMENTATION
    #include <stb_image.h>


    typedef enum {
        MODEL_SUCCESS =      0,
        MODEL_GL_ERR =      -1,
        MODEL_ASSIMP_ERR =  -2,
        MODEL_UNEXP_ALLOC = -3,
        MODEL_NO_MEM =      -4,
        MODEL_ERR =         -5,
        MODEL_STB_ERR =     -6,
    } model_error_t;

    struct Vertex {
        vec3 position;
        vec3 normal;
        vec2 texture_coordinates;
        vec3 tangent;
        vec3 bitangent;
    };
    typedef struct Vertex Vertex;

    typedef enum {
        DIFFUSE  = 0,
        SPECULAR = 1,
        NORMAL   = 2,
    } texture_t;
    
    struct Texture {
        unsigned int id;
        texture_t    type;
        char *       path;
    };
    typedef struct Texture Texture;

    typedef struct Texture_Node Texture_Node;
    struct Texture_Node {
        Texture texture;
        Texture_Node * next;
    };

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

    struct Model {
        char *         file_path;
        Mesh *         meshes;
        unsigned int   num_meshes;
        char *         directory;
        Texture_Node * loaded_textures;
    };
    typedef struct Model Model;


    model_error_t setup_model(Model * model);
    model_error_t setup_mesh(Mesh * mesh);
    model_error_t draw_mesh(Shader * shader, Mesh mesh);
    model_error_t draw_model(Shader * shader, Model model);
    model_error_t load_model(Model * model);
    model_error_t process_node(Model * model, struct aiNode * node,
                               const struct aiScene * scene, int * index);
    model_error_t process_mesh(struct aiMesh * mesh,
                               const struct aiScene * scene,
                               Mesh * out, Model * model);
    model_error_t load_material_textures(struct aiMaterial * material,
                                         enum aiTextureType type,
                                         texture_t type_name, int * count,
                                         Model * model, Texture ** out);
    model_error_t texture_from_file(char * fname, unsigned int * texture_id);
    void free_mesh(Mesh * mesh);
    void free_model(Model * model);
    void append_texture_node(Model * model, Texture_Node * new_node);
#endif
