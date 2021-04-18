#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <shader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


char fragSource[] = "../../shaders/textures/2frag";
char vertSource[] = "../../shaders/textures/vert";


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
}


float vertices[] = {
    //positions         colors              texture coordinates
	0.5f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
	0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
	-0.5f, 0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f
};


unsigned int indices[] = {
	0,1,3,
	1,2,3
};


int main(){
	//glfw initiation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfw window creation and initialization
	GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpengl", NULL, NULL);

	if (window == NULL){
		printf("Failed to create a GLFW window");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//initialize GLAD extension loader
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD");
		return -1;
	}

	//set default window size
	glViewport(0, 0, 800, 600);

	//resize window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	
	//create an EBO and transfer data
	//import note!
	//Any time you glBindBuffer an element array while a VAO
	//is bound, the VAO automatically sets its element buffer object
	//to be the bound element array. So we don't need to repeatedly
	//do this binding in the main event loop below.
	unsigned int EBO;
	glGenBuffers(1,&EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
	//Texture time
	unsigned int texture1, texture2;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	printf("Configure texture options\n");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(0);

	int width, height, nrChannels;

	printf("stbi_load\n");
	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

	if (data){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		printf("loaded texture image\n");
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
		printf("loaded awesome face image\n");
	}
	else{
		printf("Failed to load awesome face texture\n");
	}
	stbi_image_free(data);

	struct Shader * shaders = shaderInit();
	(*shaders).load( shaders, vertSource, fragSource );

	(*shaders).use( shaders );
	glUniform1i(glGetUniformLocation( (*shaders).ID, "texture1"), 0);
	glUniform1i(glGetUniformLocation( (*shaders).ID, "texture2"), 1);

	while (!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//tell opengl to use the shader program we created
		(*shaders).use( shaders );
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		processInput(window);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
