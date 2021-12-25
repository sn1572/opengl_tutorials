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
    model.directory = dirname(model.file_path);
    /* We have to explicitly cast because of g++. Hooray. */
    model.meshes = (Mesh *)malloc(scene->mNumMeshes * sizeof(Mesh));
    process_node(model, scene->mRootNode, scene, 0);
    return MODEL_SUCCESS;
}


model_error_t process_node(Model model, aiNode * node, const aiScene * scene,
                           int index)
{
    model_error_t result = MODEL_SUCCESS;
    Mesh mesh;
    aiMesh * ai_mesh = NULL;

    for (int i = 0; i < node->mNumMeshes; i++){
        /* If loading succeeded this pointer should be valid. */
        ai_mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(ai_mesh, scene, model.directory, &mesh);
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


model_error_t process_mesh(aiMesh * mesh, const aiScene * scene,
                           char * directory, Mesh * out)
{
    Vertex vertex;
    int num_indices = 0;
    aiFace face;
    int count = 0;
    aiMaterial * material;
    Texture * diffuse_maps = NULL;
    Texture * specular_maps = NULL;
    int num_diffuse_textures = 0;
    int num_specular_textures = 0;

    /* Vertex processing. More eye-rolling over explicit casts for g++. */
    out->vertices = (Vertex *)malloc(mesh->mNumVertices * sizeof(Vertex));
    if (!out->vertices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        free(out);
        out = NULL;
        return MODEL_NO_MEM;
    }
    for (int i = 0; i < mesh->mNumVertices; i++){
        vertex.position[0] = mesh->mVertices[i].x;
        vertex.position[1] = mesh->mVertices[i].y;
        vertex.position[2] = mesh->mVertices[i].z;
        vertex.normal[0] = mesh->mNormals[i].x; 
        vertex.normal[1] = mesh->mNormals[i].y;
        vertex.normal[2] = mesh->mNormals[i].z;
        /* There are up to 8 different texture coordinates per vertex.
         * For now we just load the first, if it exists.
         */
        if (mesh->mTextureCoords[0]){
            vertex.texture_coordinates[0] = mesh->mTextureCoords[0][i].x;
            vertex.texture_coordinates[1] = mesh->mTextureCoords[0][i].y;
        } else{
            vertex.texture_coordinates[0] = 0.f;
            vertex.texture_coordinates[1] = 0.f;
        }
        out->vertices[i] = vertex;
    }
    for (int i = 0; i < mesh->mNumFaces; i++){
        face = mesh->mFaces[i];
        num_indices += face.mNumIndices;
    }

    /* Index processing */
    out->indices = (unsigned int *)malloc(num_indices * sizeof(unsigned int));
    if (!out->indices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        free(out->vertices);
        free(out);
        out = NULL;
        return MODEL_NO_MEM;
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
                                              DIFFUSE, &num_diffuse_textures,
                                              directory);
        specular_maps = load_material_textures(material,
                                               aiTextureType_SPECULAR,
                                               SPECULAR,
                                               &num_specular_textures,
                                               directory);
        out->textures = (Texture *)malloc((num_diffuse_textures + \
                                           num_specular_textures) * \
                                           sizeof(Texture));
        if (!out->textures || !diffuse_maps || !specular_maps){
            fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
            free(out->vertices);
            free(out->indices);
            if (out->textures)
                free(out->textures);
            if (diffuse_maps)
                free(diffuse_maps);
            if (specular_maps)
                free(specular_maps);
            free(out);
            out = NULL;
            return MODEL_ERR;
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

    return MODEL_SUCCESS;
}


Texture * load_material_textures(aiMaterial * mat, aiTextureType type,
                                 texture_t type_name, int * count,
                                 char * directory)
{
    /* Store the number of textures found in count. */
    aiString string;
    Texture * textures = NULL;
    unsigned int texture_id;
    Texture texture;

    if (!mat->GetTextureCount(type)){
        fprintf(stderr, "%s %d: %s: Material has no textures!\n", __FILE__,
                __LINE__, __func__);
        return NULL;
    }
    textures = (Texture *)malloc(mat->GetTextureCount(type) * sizeof(Texture));
    if (!textures){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        return NULL;
    }
    for (int i = 0; i < mat->GetTextureCount(type); i++){
        mat->GetTexture(type, i, &string);
        if (!texture_from_file((char *)string.C_Str(), directory,
                               &texture_id)){
            free(textures);
            fprintf(stderr, "%s %d: Texture from file error.\n", __FILE__,
                    __LINE__);
            return NULL;
        }
        texture.id = texture_id;
        texture.type = type_name;
        texture.path = (char *)string.C_Str();
        textures[i] = texture;
    }
    return textures;
}


model_error_t texture_from_file(char * file_name, char * directory,
                                unsigned int * texture_id){
    int width, height, nrChannels;
    const int max_file_name = 256;
    char full_name[max_file_name];

    snprintf(full_name, max_file_name, "%s/%s", directory, file_name);
    printf("Texture full path:\n%s\n", full_name);
    unsigned char * data = stbi_load(full_name, &width, &height,
                                     &nrChannels, 0);
    if (data){
        GLenum format;
        switch(nrChannels){
            case 1:
                format = GL_RED;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                fprintf(stderr, "%s %d: Unrecognized number of channels: %i\n",
                        __FILE__, __LINE__, nrChannels);
                stbi_image_free(data);
                return MODEL_STB_ERR;
        }
        glGenTextures(1, texture_id);
        glBindTexture(GL_TEXTURE_2D, *texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return MODEL_SUCCESS;
    }
    else{
        fprintf(stderr, "%f %d: Failed to load texture\n", __FILE__,
                __LINE__);
        return MODEL_STB_ERR;
    }
}
