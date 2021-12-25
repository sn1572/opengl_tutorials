#include <model.h>


model_error_t setup_mesh(Mesh mesh)
{
    model_error_t result = MODEL_SUCCESS;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices * sizeof(Vertex),
                 mesh.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.num_indices * \
                 sizeof(unsigned int),
                 mesh.indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texture_coordinates));
    // This breaks the existing vertex array binding
    glBindVertexArray(0);

    #ifdef MODEL_DEBUG
    if (glGetError() != GL_NO_ERROR){
        result = MODEL_GL_ERR;
    }
    #endif
    return result;
}


model_error_t draw_mesh(Shader * shader, Mesh mesh)
{
    model_error_t result = MODEL_SUCCESS;
    unsigned int diffuse_count = 1;
    unsigned int specular_count = 1;
    /* May get quiet errors if the name string is not long enough,
     * resulting in invalid uniform names being sent to the shader.
     * The shader generally won't tell you if that happens.
     */
    const int max_unif_name = 128;
    char name[max_unif_name];

    for (unsigned int i=0; i < mesh.num_textures; i++){
        glActiveTexture(GL_TEXTURE0 + i);
        if (mesh.textures[i].type == DIFFUSE){
            snprintf(name, max_unif_name, "material.texture_diffuse%i",
                     diffuse_count++);
        } else if (mesh.textures[i].type == SPECULAR){
            snprintf(name, max_unif_name, "material.texture_specular%i",
                     specular_count++);
        }
        setInt(shader, name, i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    #ifdef MODEL_DEBUG
    if (glGetError() != GL_NO_ERROR){
        result = MODEL_GL_ERR;
    }
    #endif
    return result;
}


model_error_t draw_model(Shader * shader, Model model)
{
    model_error_t result = MODEL_SUCCESS;
    for (int i=0; i < model.num_meshes; i++){
        result = draw_mesh(shader, model.meshes[i]);
    }
    return result;
}


model_error_t load_model(Model model)
{
    /* Filthy, unclean c++. Begone, TCPPOT. */
    int count = 0;
    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(model.file_path,
                                            aiProcess_Triangulate | \
                                            aiProcess_FlipUVs | \
                                            aiProcess_GenNormals);
    if (model.meshes){
        /* This guarantees model.meshes == NULL */
        fprintf(stderr, "This model already contains data.\n");
        return MODEL_UNEXP_ALLOC;
    }
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || \
        !scene->mRootNode)
    {
        fprintf(stderr, "Assimp error: %s\n", import.GetErrorString());
        return MODEL_ASSIMP_ERR;
    }
    model.directory = path.substr(0, path.find_last_of('/'));
    model.meshes = malloc(scene->mNumMeshes * sizeof(Mesh));
    process_node(model, scene->mRootNode, scene, 0);
    return MODEL_SUCCESS
}


model_error_t process_node(Model model, aiNode * node, const aiScene * scene,
                           int index)
{
    model_error_t result = MODEL_SUCCESS;
    Mesh * mesh = NULL;
    aiMesh * ai_mesh = NULL;

    for (int i = 0; i < node->mNumMeshes; i++){
        /* If loading succeeded this pointer should be valid. */
        ai_mesh = scene->mMeshes[node->mMeshes[i]];
        mesh = calloc(1, sizeof(Mesh));
        if (!mesh){
            fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
            return MODEL_NO_MEM;
        }
        *mesh = process_mesh(ai_mesh, scene);
        model.meshes[index++] = mesh;
    }
    for (int i = 0; i < node->mNumChildren; i++){
        result = process_node(model, node->mChildren[i], scene, index);
        if (!result){
            break;
        }
    }
    return result;
}


Mesh process_mesh(aiMesh * mesh, const aiScene * scene)
{
    Mesh out;
    vec3 vector;
    vec2 vector2;
    Vertex vertex;
    int num_indices = 0;
    aiFace face;
    int count = 0;
    aiMaterial * material;
    Texture * diffuse_maps = NULL;
    Texture * specular_maps = NULL;
    int num_diffuse_textures = 0;
    int num_specular_textures = 0;

    /* Vertex processing */
    out->vertices = malloc(mesh->mNumVertices * sizeof(Vertex));
    if (!out->vertices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        free(out);
        return NULL;
    }
    for (int i = 0; i < mesh->mNumVertices; i++){
        vector = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                  mesh->mVertices[i].z};
        vertex.position = vector;
        vector = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                  mesh->mNormals[i].z};
        vertex.normal = vector;
        /* There are up to 8 different texture coordinates per vertex.
         * For now we just load the first, if it exists.
         */
        if (mesh->mTextureCoords[0]){
            vector2 = {mesh->mTextureCoords[0][i].x,
                       mesh->mTextureCoords[0][i].y};
            vertex.texture_coordinates = vector2;
        } else{
            vertex.texture_coordinates = {0.f, 0.f};
        }
        out->vertices[i] = vertex;
    }
    for (int i = 0; i < mesh->mNumFaces; i++){
        face = mesh->mFaces[i];
        num_indices += face.mNumIndices;
    }

    /* Index processing */
    out->indices = malloc(num_indices * sizeof(unsigned int));
    if (!out->indices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        free(out->vertices);
        free(out);
        return NULL;
    }
    for (int i = 0; i < mesh->mNumFaces; i++){
        face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++){
            out->indices[count++] = face.mIndices[j];
        }
    }

    /* Texture processing */
    if (mesh->mMaterialIndex >= 0){
        material = scene->mMaterials[mesh->mMaterialIndex];
        diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE,
                                              DIFFUSE, &num_diffuse_textures);
        specular_maps = load_material_textures(material,
                                               aiTextureType_SPECULAR,
                                               SPECULAR,
                                               &num_specular_textures);
        out->textures = malloc((num_diffuse_textures + \
                                num_specular_textures) * sizeof(Texture));
        if (!out->textures || !diffuse_maps || !specular_maps){
            fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
            free(out->vertices);
            free(out->indices);
            free(out);
            return NULL;
        }
        for (int i = 0; i < num_diffuse_textures; i++){
            out->textures[i] = diffuse_maps[i];
        }
        for (int i = 0; i < num_specular_textures; i++){
            out->textures[i+num_diffuse_textures] = specular_maps[i];
        }
        free(diffuse_maps);
        free(specular_maps);
    }

    return out;
}


Texture * load_material_textures(aiMaterial * mat, aiTextureType type,
                                 texture_t type_name, int * count)
{
    /* Store the number of textures found in count. */
}
