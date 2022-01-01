#include <shader.h>


#ifdef SHADER_DEBUG
#define gl_err_check(goto_loc) do {\
    if (glGetError() != GL_NO_ERROR){\
        err_print("GL err\n");\
        result = SHADER_GL_ERR;\
        goto goto_loc;\
    }} while(0)
#else
#define gl_err_check(goto_loc){}
#endif


#ifdef SHADER_DEBUG
#define gl_err_check_no_goto() do {\
    if (glGetError() != GL_NO_ERROR){\
        err_print("GL err\n");\
        result = SHADER_GL_ERR;\
    }} while(0)
#else
#define gl_err_check_no_goto(){}
#endif


shader_err_t readFile(const char * fname, char ** buffer)
{
    // buffer does not need to be pre-allocated
    FILE * fp;
    long size;

    fp = fopen(fname, "rb");
    if(!fp){
        err_print("Failure opening file\n");
        return SHADER_FS_ERR;
    }
    fseek(fp , 0L , SEEK_END);
    size = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    *buffer = calloc(size+1, sizeof(char));
    if(!buffer){
        fclose(fp);
        err_print("Failed to allocate memory\n");
        return SHADER_NO_MEM;
    }

    /* copy the file into the buffer */
    if(1 != fread(*buffer, size, 1, fp)){
        fclose(fp);
        free(*buffer);
        err_print("File read failure\n");
        return SHADER_FS_ERR;
    }
    // NUL terminate the buffer
    (*buffer)[size] = '\0';
    fclose(fp);
    return SHADER_NO_ERR;
}


shader_err_t load(struct Shader * self, char * vertexPath, char * fragmentPath)
{
    char * vertexSource = NULL;
    char * fragmentSource = NULL;
    unsigned int vertex, fragment;
    shader_err_t result = SHADER_NO_ERR;

    result = readFile(vertexPath, &vertexSource);
    if (result != SHADER_NO_ERR)
        return result;
    if (!(vertexSource)){
        err_print("Vertex shader file read failure\n");
        return SHADER_NULL_PTR;
    }
    result = readFile(fragmentPath, &fragmentSource);
    if (result != SHADER_NO_ERR){
        goto free_1;
    }
    if (!(fragmentSource)){
        err_print("Fragment shader file read failure\n");
        result = SHADER_NULL_PTR;
        goto free_1;
    }

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    gl_err_check(free_2);
    glShaderSource(vertex, 1, (const char **)(&vertexSource), NULL);
    gl_err_check(shader_1);
    glCompileShader(vertex);
    gl_err_check(shader_1);
    checkCompileErrors(vertex, "VERTEX");

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    gl_err_check(shader_1);
    glShaderSource(fragment, 1, (const char **)(&fragmentSource), NULL);
    gl_err_check(shader_2);
    glCompileShader(fragment);
    gl_err_check(shader_2);
    checkCompileErrors(fragment, "FRAGMENT");

    /* shader Program */
    GLint ID = glCreateProgram();
    #ifdef SHADER_DEBUG
    if (glGetError() != GL_NO_ERROR){
        err_print("Failed to create shader program\n");
        result = SHADER_GL_ERR;
        goto shader_2;
    }
    #endif
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    #ifdef SHADER_DEBUG
    if (checkCompileErrors(ID, "PROGRAM") != SHADER_NO_ERR){
        err_print("Shader compilation failed\n");
        result = SHADER_GL_ERR;
        glDeleteProgram(ID);
        goto shader_2;
    }
    #endif
    /* The penultimate assignment. If we got here, we succeeded. */
    self->ID = ID;

    /* Cleanup */
    shader_2:
        glDeleteShader(fragment);
    shader_1:
        glDeleteShader(vertex);
    free_2:
        free(fragmentSource);
    free_1:
        free(vertexSource);
    return result;
}


shader_err_t use(struct Shader * self)
{
    shader_err_t result = SHADER_NO_ERR;
    glUseProgram(self->ID); 
    gl_err_check_no_goto();
    return result;
}


shader_err_t setBool(struct Shader * self, const char * name, int value)
{
    shader_err_t result = SHADER_NO_ERR;
    glUniform1i(glGetUniformLocation(self->ID, name), (int)value);
    gl_err_check_no_goto();
    return result;
}


shader_err_t setInt(struct Shader * self, const char * name, int value)
{
    shader_err_t result = SHADER_NO_ERR;
    glUniform1i(glGetUniformLocation((*self).ID, name), value); 
    gl_err_check_no_goto();
    return result;
}


shader_err_t setFloat(struct Shader * self, const char * name, float value)
{
    shader_err_t result = SHADER_NO_ERR;
    glUniform1f(glGetUniformLocation((*self).ID, name), value); 
    gl_err_check_no_goto();
    return result;
}


shader_err_t setVec3(struct Shader * self, const char * name, vec3 vec)
{
    shader_err_t result = SHADER_NO_ERR;
    glUniform3f(glGetUniformLocation((*self).ID, name), vec[0], vec[1], vec[2]);
    gl_err_check_no_goto();
    return result;
}


shader_err_t flatten(float * out, mat4x4 M)
{
    if (!out)
        return SHADER_NULL_PTR;
    if (!M)
        return SHADER_NULL_PTR;
    for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            out[i*4+j] = M[i][j];
        }
    }
    return SHADER_NO_ERR;
}


shader_err_t setMat4x4(struct Shader * shaders, const char * name,
                       mat4x4 matrix)
{
    GLint loc = glGetUniformLocation(shaders->ID, name);
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "%s %d: GL error %x\n", __FILE__, __LINE__, glError);
        return SHADER_GL_ERR;
    }
    float flattened[16];
    flatten(flattened, matrix);
    glUniformMatrix4fv(loc, 1, GL_FALSE, flattened);
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "%s %d: GL error %x\n", __FILE__, __LINE__, glError);
        return SHADER_GL_ERR;
    }
}


struct Shader * shaderInit()
{
    struct Shader * out = NULL;
    out = calloc(1, sizeof(struct Shader));
    if (!(out)){
        err_print("enomem\n");
        return out;
    }
    (*out).load = load;
    (*out).use = use;
    (*out).setBool = setBool;
    (*out).setInt = setInt;
    (*out).setFloat = setFloat;
    (*out).setVec3 = setVec3;
    (*out).self = out;
    return out;
}


shader_err_t checkCompileErrors(unsigned int shader, char * type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM"){
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "SHADER_COMPILATION_ERROR of type: %s\n%s\n",
                    type, infoLog);
            return SHADER_GL_ERR;
        }
    }
    else{
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fprintf(stderr, "PROGRAM_LINKING_ERROR of type: %s\n%s\n",
                    type, infoLog);
            return SHADER_GL_ERR;
        }
    }
    return SHADER_NO_ERR;
}


void shader_introspection(struct Shader * shader)
{
    /* Stolen from Stack Overflow.
     * Super useful for inspecting the internal state
     * of a given shader and for checking if the shader is currently active.
     */
    unsigned int active_program = shader->ID;
    GLint i;
    GLint count;
    GLint size;
    GLenum type;
    const GLsizei bufSize = 16;
    GLchar name[bufSize];
    GLsizei length;
    GLint program_valid = 0;
    GLint current_program = 0;
    GLint num_active_uniforms = 0;

    if (!glIsProgram(active_program)) {
        printf("Active program is not valid.\n");
    }

    glValidateProgram(active_program);
    glGetProgramiv(active_program, GL_VALIDATE_STATUS, &program_valid);
    if (GL_TRUE != program_valid) {
        printf("Program validation failed.\n");
    }

    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    if (0 == current_program) {
        printf("Error, no current program is set.\n");
    } else if (current_program != active_program) {
        printf("Error, current program doesn't match active_program!\n");
    }

    glGetProgramiv(active_program, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
    if (0 == num_active_uniforms) {
        printf("There are 0 uniforms active in program %d.\n", active_program);
    } else {
        printf("There are %d uniform(s) active in program %d.\n",
               num_active_uniforms, active_program);
    }

    for (i = 0; i < num_active_uniforms; i++){
        glGetActiveUniform(active_program, (GLuint)i, bufSize, &length,
                           &size, &type, name);
        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }
}
