#ifndef MODEL_H
    #define MODEL_H

    #include <linmath.h>
    #include <stdio.h>
    #include <shader.h>
    #include <GLFW/glfw3.h>
    #include <stddef.h>
    #include <assimp/scene.h>
    #include <assimp/postprocess.h>
    #include <assimp/Importer.hpp>

    typedef enum {
        MODEL_SUCCESS =  0,
        MODEL_NO_MEM =   -1,
        MODEL_NULL_PTR = -2,
        MODEL_GL_ERR =   -3,
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

    struct Model {
        char *       file_path;
        Mesh *       meshes;
        unsigned int num_meshes;
        char *       directory;
    };
    typedef struct Model Model;

    model_error_t setup_mesh(Mesh mesh);
    model_error_t draw_mesh(Shader * shader, Mesh mesh);
    model_error_t load_model(Model model);
    model_error_t process_node(aiNode * node, const aiScene * scene);
    Mesh process_mesh(aiMesh * mesh, const aiScene * scene);
    Texture * load_material_textures(aiMaterial * material, aiTextureType type,
                                     char * type_name);
    model_error_t draw_model(Shader * shader, Model model);
#endif
