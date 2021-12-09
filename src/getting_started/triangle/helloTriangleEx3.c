#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>


static const char *vertexShaderSource = "#version 330 core\n"
"layout (location=0) in vec3 aPos;\n"
"void main(){\n"
"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";


static const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main(){\n"
"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";


static const char *fragmentShaderSourceYellow = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main(){\n"
"	FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
"}\0";


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}


void processInput(GLFWwindow *window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
}


float vertices[] = {
	-0.1f, -0.1f, 0.0f,
    0.1f, -0.1f, 0.0f,
    0.0f,  0.1f, 0.0f
};


float vertices2[] = {
	0.1f, -0.1f, 0.0f,
    0.3f, -0.1f, 0.0f,
   0.2f,  0.1f, 0.0f
};


int main(){
	//glfw initiation
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int WIDTH = 800;
	int HEIGHT = 600;

	//glfw window creation and initialization
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpengl", NULL, NULL);

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
	glViewport(0, 0, WIDTH, HEIGHT);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	unsigned int VAO2;
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);

	unsigned int VBO2;
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	//compile the vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//Check for failure
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Error. Vertex shader compilation failed.\n");
		printf(infoLog);
	}

	//compile the fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//Check for failure
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("Error. Fragment shader compilation failed.\n");
		printf(infoLog);
	}

	/*
	//I guess we need a separate vertex shader for shaderProgramYellow
	unsigned int vertexShaderYellow;
	vertexShaderYellow = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderYellow, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderYellow);

	glGetShaderiv(vertexShaderYellow, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertexShaderYellow, 512, NULL, infoLog);
		printf("Error. Vertex shader compilation failed.\n");
		printf(infoLog);
	}
	*/

	unsigned int fragmentShaderYellow;
	fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderYellow, 1, &fragmentShaderSourceYellow, NULL);
	glCompileShader(fragmentShaderYellow);

	glGetShaderiv(fragmentShaderYellow, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragmentShaderYellow, 512, NULL, infoLog);
		printf("Error. Fragment shader compilation failed.\n");
		printf(infoLog);
	}

	//Create a shader program object
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//check for failure
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}

	//Create a second shader program object
	unsigned int shaderProgramYellow;
	shaderProgramYellow = glCreateProgram();
	glAttachShader(shaderProgramYellow, vertexShader);
	glAttachShader(shaderProgramYellow, fragmentShaderYellow);
	glLinkProgram(shaderProgramYellow);

	//check for failure
	glGetProgramiv(shaderProgramYellow, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(shaderProgramYellow, 512, NULL, infoLog);
	}

	//Apparently we don't need the shader objects they've been linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(fragmentShaderYellow);

	while (!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//tell opengl to use the shader program we created
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(shaderProgramYellow);
		glBindVertexArray(VAO2);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		processInput(window);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
