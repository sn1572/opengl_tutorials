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
