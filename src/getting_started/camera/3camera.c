#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <shader.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../../../linmath/linmath.h"
#include <camera.h>


#define _WIDTH 1920
#define _HEIGHT 1080


char fragSource[] = "../../shaders/camera/frag";
char vertSource[] = "../../shaders/camera/vert";


// file scope variables for storing UI state 
static int WIDTH = _WIDTH;
static int HEIGHT = _HEIGHT;


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}


float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};


float cubePositions[] = {
	0.0f,  0.0f,  0.0f, 
	2.0f,  5.0f, -15.0f, 
	-1.5f, -2.2f, -2.5f,  
	-3.8f, -2.0f, -12.3f,  
	2.4f, -0.4f, -3.5f,  
	-1.7f,  3.0f, -7.5f,  
	1.3f, -2.0f, -2.5f,  
	1.5f,  2.0f, -2.5f, 
	1.5f,  0.2f, -1.5f, 
	-1.3f,  1.0f, -1.5f  
};


unsigned int indices[] = {
	0,1,3,
	1,2,3
};


void flatten( float * out, mat4x4 M ){
	for (int i=0; i<4; i++){
		for (int j=0; j<4; j++){
			out[i*4+j] = M[i][j];
		}
	}
}


void sendMatrixToShader( mat4x4 matrix, const char * name, struct Shader * shaders ){
	GLint loc = glGetUniformLocation( shaders->ID, name );
	float flattened[16];
	flatten( flattened, matrix );
	glUniformMatrix4fv( loc, 1, GL_FALSE, flattened );
	if (glGetError() != GL_NO_ERROR){
		fputs("Failed transferring matrix to shader program", stderr);
		printf("Failed transffering to shader uniform %s\n", name);
		exit(1);
	}
}


int main(){
	//glfw initiation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfw window creation and initialization
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpengl", NULL, NULL);

	if (window == NULL){
		printf("Failed to create a GLFW window");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//initialize GLAD loader
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD");
		return -1;
	}

	// set default window size
	glViewport(0, 0, WIDTH, HEIGHT);

	// Init the camera `object` and hook its methods into the callbacks
	struct Camera * cam;
	cam = cameraInit( WIDTH, HEIGHT );
	setActiveCamera( cam );
	setActiveCameraPosition( 0, 0, 3 );

	// frame buffer size callback
	glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

	// Track mouse callback
	glfwSetCursorPosCallback( window, glfwCompatMouseMovementCallback );

	// Scroll wheel callback
	glfwSetScrollCallback( window, glfwCompatMouseScrollCallback );

	glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

	//create a VAO for VBO record keeping
	//The first argument to glVertexAttribPointer is the "position" label
	//that we referenced in the vertex shader with layout (location=0)
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);	

	//create a VBO and transfer data
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//bind VBO to VAO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
    
	//Texture time
	unsigned int texture1, texture2;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(0);

	int width, height, nrChannels;

	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

	if (data){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else{
		printf("Failed to load texture\n");
	}
	stbi_image_free(data);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(1);

	data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);

	if (data){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else{
		printf("Failed to load awesome face texture\n");
	}
	stbi_image_free(data);

	struct Shader * shaders = shaderInit();
	shaders->load( shaders, vertSource, fragSource );
	shaders->use( shaders );

	glUniform1i(glGetUniformLocation( (*shaders).ID, "texture1"), 0);
	glUniform1i(glGetUniformLocation( (*shaders).ID, "texture2"), 1);

	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glEnable(GL_DEPTH_TEST);

	int numFrames = 0;
	float past = (float)glfwGetTime();

	while (!glfwWindowShouldClose(window)){
		numFrames += 1;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cam->setViewMatrix( cam, shaders, "view" );
		cam->setProjectionMatrix( cam, shaders, "projection" );
		glfwCompatKeyboardCallback( window );
		
		for (int i=0; i<10; i++){
			float x, y, z;
			x = cubePositions[3*i];
			y = cubePositions[3*i+1];
			z = cubePositions[3*i+2];

			mat4x4 model;
			mat4x4_translate(model, x, y, z);
			if (i%3 == 0){
				float angle = (float)glfwGetTime()*50*M_PI/180;
				float offset = 20*i;
				mat4x4_rotate(model, model, 0.5, 1, 0, angle+offset);
			}
			else{
				mat4x4_rotate(model, model, 0.5, 1, 0, 0);
			}
			sendMatrixToShader( model, "model", shaders );
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	float time = (float)glfwGetTime();
	printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n", numFrames, time, numFrames/time);

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}
