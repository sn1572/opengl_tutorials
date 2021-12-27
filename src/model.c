#include <model.h>


model_error_t setup_model(Model * model)
{
    model_error_t result = MODEL_SUCCESS;

    for (int i = 0; i < model->num_meshes; i++){
        result = setup_mesh(model->meshes+i);
    }
    return result;
}


model_error_t setup_mesh(Mesh * mesh)
{
    /* Currently there is no cleanup for the buffers and arrays
     * allocated below if this function fails...
     * It is also not currently possible to tell if they've even
     * been allocated. As a result it is currently best to terminate
     * program execution if setup_mesh fails.
     * One "Easy" way that may not be thread-safe is to call glGetError()
     * at the top of this function and then after every gl* call.
     * TODO: Fix this if possible or implement a gl error hook.
     */
    model_error_t result = MODEL_SUCCESS;

    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(Vertex),
                 mesh->vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * \
                 sizeof(unsigned int),
                 mesh->indices, GL_STATIC_DRAW);

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
        switch(mesh.textures[i].type){
            case DIFFUSE:
                snprintf(name, max_unif_name, "material.texture_diffuse%i",
                         diffuse_count++);
                break;
            case SPECULAR:
                snprintf(name, max_unif_name, "material.texture_specular%i",
                         specular_count++);
                break;
            default:
                fprintf(stderr, "%s %d: Texture type unrecognized: %i\n",
                        __FILE__, __LINE__, mesh.textures[i].type);
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


model_error_t load_model(Model * model)
{
    /* model is expected to come with a valid file_path value */
    model_error_t result = MODEL_SUCCESS;
    int count = 0;
    const struct aiScene * scene;

    if (model->meshes || model->loaded_textures){
        /* This guarantees model.meshes == NULL and 
         * model.loaded_textures == NULL
         */
        fprintf(stderr, "Ensure that model->meshes is NULL and \
                model->loaded_textures is NULL.\n");
        return MODEL_UNEXP_ALLOC;
    }
    scene = aiImportFile(model->file_path, aiProcess_Triangulate | \
                         aiProcess_FlipUVs | aiProcess_GenNormals);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || \
        !scene->mRootNode)
    {
        fprintf(stderr, "Some kind of assimp error.\n");
        return MODEL_ASSIMP_ERR;
    }
    /* dirname may alter the passed file path...and it can segfault
     * if you try to alter a string literal (aka, something defined
     * by "this syyntax". To avoid this always initialized model path
     * like so:
     * char file[] = "some/path";
     * model.file_path = file;
     */
    model->directory = dirname(model->file_path);
    model->meshes = malloc(scene->mNumMeshes * sizeof(Mesh));
    model->num_meshes = scene->mNumMeshes;
    model->loaded_textures = NULL;
    result = process_node(model, scene->mRootNode, scene, &count);
    return result;
}


model_error_t process_node(Model * model, struct aiNode * node,
                           const struct aiScene * scene, int * index)
{
    model_error_t result = MODEL_SUCCESS;
    Mesh mesh;
    struct aiMesh * ai_mesh = NULL;

    for (int i = 0; i < node->mNumMeshes; i++){
        /* If loading succeeded this pointer should be valid. */
        ai_mesh = scene->mMeshes[node->mMeshes[i]];
        result = process_mesh(ai_mesh, scene, &mesh, model);
        if (result){
            /* Note: If an error occurred the contents of mesh are garbage. */
            fprintf(stderr, "%s %d: Process_mesh failure during %s. \
                    Model data incomplete.\n", __FILE__, __LINE__, __func__);
            return result;
        }
        model->meshes[(*index)++] = mesh;
    }
    for (int i = 0; i < node->mNumChildren; i++){
        result = process_node(model, node->mChildren[i], scene, index);
        if (result){
            return result;
        }
    }
    return result;
}


model_error_t process_mesh(struct aiMesh * mesh, const struct aiScene * scene,
                           Mesh * out, Model * model)
{
    Vertex vertex;
    int num_indices = 0;
    struct aiFace face;
    int count = 0;
    struct aiMaterial * material;
    Texture * diffuse_maps = NULL;
    Texture * specular_maps = NULL;
    int num_diffuse_textures = 0;
    int num_specular_textures = 0;

    /* Vertex processing. */
    out->vertices = malloc(mesh->mNumVertices * sizeof(Vertex));
    if (!out->vertices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        return MODEL_NO_MEM;
    }
    out->num_vertices = mesh->mNumVertices;
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

    /* Index processing */
    for (int i = 0; i < mesh->mNumFaces; i++){
        face = mesh->mFaces[i];
        num_indices += face.mNumIndices;
    }
    out->indices = (unsigned int *)malloc(num_indices * sizeof(unsigned int));
    if (!out->indices){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        free(out->vertices);
        out->vertices = NULL;
        return MODEL_NO_MEM;
    }
    out->num_indices = num_indices;
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
                                              model);
        specular_maps = load_material_textures(material,
                                               aiTextureType_SPECULAR,
                                               SPECULAR,
                                               &num_specular_textures,
                                               model);
        out->textures = malloc((num_diffuse_textures + \
                                num_specular_textures) * sizeof(Texture));
        if (!out->textures || !diffuse_maps || !specular_maps){
            fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
            free(out->vertices);
            out->vertices = NULL;
            free(out->indices);
            out->vertices = NULL;
            if (out->textures){
                free(out->textures);
                out->textures = NULL;
            }
            if (diffuse_maps)
                free(diffuse_maps);
            if (specular_maps)
                free(specular_maps);
            return MODEL_ERR;
        }
        out->num_textures = num_diffuse_textures + num_specular_textures;
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


Texture * load_material_textures(struct aiMaterial * mat,
                                 enum aiTextureType type,
                                 texture_t type_name, int * count,
                                 Model * model)
{
    /* Store the number of textures found in count. */
    struct aiString string;
    Texture * textures = NULL;
    unsigned int texture_id;
    Texture texture;
    Texture_Node * node;
    const int max_file_name = 256;
    char file_name[max_file_name];
    char compare_name[max_file_name];
    int load_texture;
    Texture_Node * new_node = NULL;
    int string_length;

    if (!aiGetMaterialTextureCount(mat, type)){
        fprintf(stderr, "%s %d: %s: Material has no textures!\n", __FILE__,
                __LINE__, __func__);
        return NULL;
    }
    textures = malloc(aiGetMaterialTextureCount(mat, type) * sizeof(Texture));
    if (!textures){
        fprintf(stderr, "%s %d: Out of memory.\n", __FILE__, __LINE__);
        return NULL;
    }
    for (unsigned int i = 0; i < aiGetMaterialTextureCount(mat, type); i++){
        /* arg i to aiGetMaterialTexture needs to be a uint */
        load_texture = 1;
        aiGetMaterialTexture(mat, type, i, &string, NULL, NULL, NULL, NULL,
                             NULL, NULL);
        string_length = snprintf(file_name, max_file_name, "%s/%s",
                                 model->directory, string.data);
        if (string_length >= max_file_name || string_length < 0){
            fprintf(stderr, "snprintf error.\n");
            return NULL;
        }
        for (node = model->loaded_textures; node; node = node->next){
            if (!strcmp(file_name, (node->texture).path)){
                texture_id = (node->texture).id;
                load_texture = 0;
                break;
            }
        }
        if (load_texture){
            if (texture_from_file(file_name, &texture_id)){
                free(textures);
                fprintf(stderr, "%s %d: Texture from file error.\n", __FILE__,
                        __LINE__);
                return NULL;
            }
        }
        texture.id = texture_id;
        texture.type = type_name;
        texture.path = file_name;
        textures[i] = texture;
        if (load_texture){
            new_node = malloc(sizeof(Texture_Node));
            new_node->texture = texture;
            new_node->next = NULL;
            append_texture_node(model, new_node);
        }
    }
    return textures;
}


model_error_t texture_from_file(char * file_name, unsigned int * texture_id)
{
    int width, height, nrChannels;
    GLenum format;

    unsigned char * data = stbi_load(file_name, &width, &height,
                                     &nrChannels, 0);
    if (data){
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        return MODEL_SUCCESS;
    }
    else{
        fprintf(stderr, "%f %d: Failed to load texture\n", __FILE__,
                __LINE__);
        return MODEL_STB_ERR;
    }
}


void free_mesh(Mesh * mesh)
{
    if (mesh->vertices){
        free(mesh->vertices);
        mesh->vertices = NULL;
    }
    if (mesh->indices){
        free(mesh->indices);
        mesh->indices = NULL;
    }
    if (mesh->textures){
        free(mesh->textures);
        mesh->textures = NULL;
    }
    /* Because of these calls, it is only safe to use mesh_free on a mesh
     * that has successfully passed through both process_mesh and setup_mesh.
     */
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    glDeleteVertexArrays(1, &mesh->VAO);
    for (int i = 0; i < mesh->num_textures; i++){
        glDeleteTextures(1, &(mesh->textures[i].id));
    }
}


void free_model(Model * model)
{
    /* As with free_mesh, this is only safe to use on models which
     * have survived load_model in its entirety.
     */
    for (int i = 0; i < model->num_meshes; i++){
        free_mesh(&(model->meshes[i]));
    }
    free(model->meshes);
}


void append_texture_node(Model * model, Texture_Node * new_node)
{
    Texture_Node * current_node;

    /* Just to be safe. */
    new_node->next = NULL;
    if (!model->loaded_textures){
        model->loaded_textures = new_node;
        return;
    }
    current_node = model->loaded_textures;
    while (current_node->next){
        current_node = current_node->next;
    }
    current_node->next = new_node;
}
