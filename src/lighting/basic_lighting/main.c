#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <shader.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../../../headers/linmath.h"
#include <camera.h>
#include "models/cube_vertices.h"
#include "models/light_vertices.h"


static const char cube_frag_source[] = "shaders/cube_frag";
static const char cube_vert_source[] = "shaders/cube_vert";
static const char light_frag_source[] = "shaders/light_frag";
static const char light_vert_source[] = "shaders/light_vert";
static int WIDTH = 1920;
static int HEIGHT = 1080;


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}


const float cube_vertices[] = {
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


const vec3 light_position = {5,5,0};


int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfw window creation and initialization
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpengl", NULL, NULL);

    if (window == NULL){
        printf("Failed to create a GLFW window");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);

    //initialize GLAD loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        printf("Failed to initialize GLAD");
        exit(1);
    }

    // set default window size
    glViewport(0, 0, WIDTH, HEIGHT);

    // Init the camera `object` and hook its methods into the callbacks
    struct Camera * cam;
    cam = cameraInit(WIDTH, HEIGHT);
    setActiveCamera(cam);
    setActiveCameraPosition(0, 0, 3);

    // frame buffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Track mouse callback
    // Function comes from the camera module
    glfwSetCursorPosCallback(window, glfwCompatMouseMovementCallback);

    // Scroll wheel callback
    // Function comes from the camera module
    glfwSetScrollCallback(window, glfwCompatMouseScrollCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // The light's VAO
    unsigned int light_VAO;
    glGenVertexArrays(1, &light_VAO);
    glBindVertexArray(light_VAO);

    // The light's VBO
    unsigned int light_VBO;
    glGenBuffers(1, &light_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, light_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), 
             light_vertices, GL_STATIC_DRAW);

    // Specification of layout for light's VAO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                  3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // VAO for cubes
    unsigned int cube_VAO;
    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);    

    //create a VBO and transfer data
    unsigned int cube_VBO;
    glGenBuffers(1, &cube_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), 
             cube_vertices, GL_STATIC_DRAW);

    // bind VBO to VAO
    // vertex coordinate layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                  5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                  5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    //Texture time
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    unsigned char *data = stbi_load("container.jpg", &width, &height,
                                    &nrChannels, 0);

    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        printf("Failed to load texture\n");
        exit(1);
    }
    stbi_image_free(data);

    struct Shader * light_shaders = shaderInit();
    light_shaders->load(light_shaders, light_vert_source, light_frag_source);
    light_shaders->use(light_shaders);
    light_shaders->setVec3(light_shaders, "light_color", 0.0f, 1.0f, 1.0f);

    struct Shader * cube_shaders = shaderInit();
    cube_shaders->load(cube_shaders, cube_vert_source, cube_frag_source);
    cube_shaders->use(cube_shaders);
    cube_shaders->setVec3(cube_shaders, "light_color", 0.0f, 1.0f, 1.0f);
    glUniform1i(glGetUniformLocation(cube_shaders->ID, "texture1"), 0);

    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "Error on line %d in file %s\n",
                __LINE__, __FILE__);
        fprintf(stderr, "GL error %x\n", glError); 
        exit(1);
    }

    /* shader introspection */
    light_shaders->use(light_shaders);
    shader_introspection(light_shaders);

    glBindVertexArray(cube_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glEnable(GL_DEPTH_TEST);

    int numFrames = 0;
    float past = (float)glfwGetTime();
    mat4x4 model;

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwCompatKeyboardCallback(window);

        // draw the light
        light_shaders->use(light_shaders);
        glBindVertexArray(light_VAO);
        cam->setViewMatrix(cam, light_shaders, "view");
        cam->setProjectionMatrix(cam, light_shaders, "projection");
        mat4x4_translate(model, light_position[0],
                         light_position[1], light_position[2]);
        sendMatrixToShader(model, "model", light_shaders);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw cubes
        cube_shaders->use(cube_shaders);
        cam->setViewMatrix(cam, cube_shaders, "view");
        cam->setProjectionMatrix(cam, cube_shaders, "projection");
        glBindVertexArray(cube_VAO);
        
        for (int i=0; i<10; i++){
            float x, y, z;
            x = cubePositions[3*i];
            y = cubePositions[3*i+1];
            z = cubePositions[3*i+2];
            mat4x4_translate(model, x, y, z);
            if (i%3 == 0){
                float angle = (float)glfwGetTime()*50*M_PI/180;
                float offset = 20*i;
                mat4x4_rotate(model, model, 0.5, 1, 0, angle+offset);
            }
            else{
                mat4x4_rotate(model, model, 0.5, 1, 0, 0);
            }
            sendMatrixToShader(model, "model", cube_shaders);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    float time = (float)glfwGetTime();
    printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n",
           numFrames, time, numFrames/time);

    glDeleteVertexArrays(1, &light_VAO);
    glDeleteBuffers(1, &light_VBO);
    glDeleteVertexArrays(1, &cube_VAO);
    glDeleteBuffers(1, &cube_VBO);

    glfwTerminate();
    return 0;
}
