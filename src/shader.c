#include <shader.h>
#include <stdio.h>


void readFile(const char * fname, char ** buffer)
{
	// buffer does not need to be pre-allocated
	FILE * fp;
	long size;
	fp = fopen (fname, "rb");
	if(!fp){
		perror("readFile");
		exit(1);
	}
	fseek(fp , 0L , SEEK_END);
	size = ftell(fp);
	rewind(fp);

    if (buffer && *buffer){
        perror("Non-empty buffer in readFile. Freeing previous content.");
        free(*buffer);
    }
	/* allocate memory for entire content */
	*buffer = calloc(size+1, sizeof(char));
	if(!(*buffer)){
		fclose(fp);
		fprintf(stderr, "Failed to allocate memory for read of "
				"file %s in function %s, line %d of file %s\n",
			    fname, __func__, __LINE__, __FILE__);
		exit(1);
	}

	/* copy the file into the buffer */
	if(1 != fread(*buffer, size, 1, fp)){
		fclose(fp);
		free(*buffer);
		fputs("File read failure", stderr);
		exit(1);
	}
	// NUL terminate the buffer
	(*buffer)[size] = '\0';
	fclose(fp);
}


void load(struct Shader * self, char * vertexPath, char * fragmentPath)
{
    char ** vertexSource = NULL;
    char ** fragmentSource = NULL;
    unsigned int vertex, fragment;

	readFile(vertexPath, vertexSource);
	if (!(vertexSource)){
		fputs("Vertex shader file read failure", stderr);
		exit(1);
	}
	readFile(fragmentPath, fragmentSource);
	if (!(fragmentSource)){
		fputs("Fragment shader file read failure", stderr);
		exit(1);
	}
    const char ** vShaderCode = vertexSource;
    const char ** fShaderCode = fragmentSource;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
	if (glGetError() != GL_NO_ERROR){
		printf("shader creation failed\n");
	}
    glShaderSource(vertex, 1, vShaderCode, NULL);
	if (glGetError() != GL_NO_ERROR){
		printf("shader source assignment failed\n");
	}
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader Program
    GLint ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
	(*self).ID = ID;
    /* delete the shaders as they're linked into our
     * program now and no longer necessary
     */
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (vertexSource)
	    free(vertexSource);
    if (fragmentSource)
	    free(fragmentSource);
}


void use(struct Shader * self)
{
    glUseProgram((*self).ID); 
}


void setBool(struct Shader * self, const char * name, int value)
{
	glUniform1i(glGetUniformLocation((*self).ID, name), (int)value); 
}


void setInt(struct Shader * self, const char * name, int value)
{
	glUniform1i(glGetUniformLocation((*self).ID, name), value); 
}


void setFloat(struct Shader * self, const char * name, float value)
{
	glUniform1f(glGetUniformLocation((*self).ID, name), value); 
}


void setVec3(struct Shader * self, const char * name, float x, float y,
    float z)
{
	//float vec[] = {x,y,z};
	glUniform3f(glGetUniformLocation((*self).ID, name), x, y, z); 
}


struct Shader * shaderInit()
{
	struct Shader * out = NULL;
	out = calloc(1, sizeof(struct Shader));
	if (out == NULL){
		fprintf(stderr, "calloc failure in %s, file %s\n", __func__, __FILE__);
		exit(1);
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


void checkCompileErrors(unsigned int shader, char * type)
{
	int success;
    char infoLog[1024];
    if (type != "PROGRAM"){
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success){
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("SHADER_COMPILATION_ERROR of type: %s\n%s\n",
                   type, infoLog);
        }
    }
    else{
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success){
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            printf("PROGRAM_LINKING_ERROR of type: %s\n%s\n",
                   type, infoLog);
        }
    }
};


void shader_introspection(struct Shader * shaders)
{
	// Stolen from Stack Overflow
	unsigned int active_program = shaders->ID;

	if (!glIsProgram(active_program)) {
	    printf("Active program is not valid.\n");
	    exit(1);
	}

	GLint program_valid = 0;
	glValidateProgram(active_program);
	glGetProgramiv(active_program, GL_VALIDATE_STATUS, &program_valid);
	if (GL_TRUE != program_valid) {
	    printf("Program validation failed.\n");
	    exit(1);
	}

	GLint current_program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
	if (0 == current_program) {
	    printf("Error, no current program is set.\n");
	    exit(1);
	} else if (current_program != active_program) {
	    printf("Error, current program doesn't match active_program!\n");
	}

	GLint num_active_uniforms = 0;
	glGetProgramiv(active_program, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	if (0 == num_active_uniforms) {
	    printf("There are 0 uniforms active in program %d.\n", active_program);
	    exit(1);
	} else {
	    printf("There are %d uniform(s) active in program %d.\n",
               num_active_uniforms, active_program);
	}

	GLint i;
	GLint count;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 16; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

	for (i = 0; i < num_active_uniforms; i++)
	{
	    glGetActiveUniform(active_program, (GLuint)i, bufSize, &length,
                           &size, &type, name);
	    printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
	}
}
