#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <shader.h>
#include <stdbool.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../../../headers/linmath.h"
#include "../../../headers/linmath_extension.h"
#include <camera.h>
#include "models/combined_cube_vertices.h"
#include "models/light_vertices.h"
#include "../../../headers/shader.h"


#define SUCCESS 0;
#define FAILURE 1;


static char cube_frag_source[] = "shaders/cube_frag";
static char cube_vert_source[] = "shaders/cube_vert";
//static char cube_frag_source[] = "shaders/simple_frag";
//static char cube_vert_source[] = "shaders/simple_vert";
static char light_frag_source[] = "shaders/light_frag";
static char light_vert_source[] = "shaders/light_vert";
static int WIDTH = 1920;
static int HEIGHT = 1080;


float * join(float * a, int rows_a, int cols_a, float * b, int rows_b,
             int cols_b)
{
    /* concatenates b to a column-wise.
     */
    float * out = NULL;
    int i, j;
    out = calloc(rows_a*(cols_a+cols_b), sizeof(float));
    if (!out){
        fprintf(stderr, "%s %d: out of mem\n", __FILE__, __LINE__);
        return NULL;
    }
    if (rows_a != rows_b){
        fprintf(stderr, "%s %d: Arrays must have same number of rows\n",
                __FILE__, __LINE__);
        free(out);
        return NULL;
    }
    for (i=0; i<rows_a; i++){
        for (j=0; j<cols_a; j++){
            out[i*(cols_a+cols_b)+j] = a[i*cols_a+j];
        }
        for (j=0; j<cols_b; j++){
            out[i*(cols_a+cols_b)+j+cols_a] = b[i*cols_b+j];
        }
    }
    return(out);
}


void print_array(float * array, int rows, int cols){
    for (int i=0; i<rows; i++){
        for (int j=0; j<cols; j++){
            printf("%4.2f ", array[i*cols+j]);
        }
        printf("\n");
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}


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


vec3 light_position = {5,5,0};


int main(){
    int status = SUCCESS;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfw window creation and context initialization
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpengl", NULL,
                                          NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to create a GLFW window\n");
        status = FAILURE;
        goto cleanup_glfw;
    }
    glfwMakeContextCurrent(window);

    //initialize GLAD loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        fprintf(stderr, "Failed to initialize GLAD\n");
        status = FAILURE;
        goto cleanup_glfw;
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
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), 
    //             cube_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* Arguments are: index in VAO, number of elements, element dtype,
     * something that's always False (lol), stride, offset
     * cube_vertices: 6*6 rows. Columns are v_x, v_y, v_z,
     * n_x, n_y, n_z, t_x, t_y.
     * v_* are vertex coordinates, n_* are normals, t_*
     * are texture coordinates.
     */
    // Vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Surface normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float),
                          (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float),
                          (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    if (glGetError() != GL_NO_ERROR){
        fprintf(stderr, "Error during VAO / VBO assignments\n");
        goto cleanup_gl;
    }
    
    //Texture time
    unsigned int diffuse_map;
    glGenTextures(1, &diffuse_map);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char * data = stbi_load("models/container2.png", &width, &height,
                                     &nrChannels, 0);
    if (data){
        GLenum format;
        switch(nrChannels){
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                fprintf(stderr, "%s %d: Unrecognized number of channels: %i\n",
                        __FILE__, __LINE__, nrChannels);
                goto cleanup_gl;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        printf("Failed to load texture\n");
        goto cleanup_gl;
    }
    stbi_image_free(data);

    struct Shader * light_shaders = shaderInit();
    if (light_shaders->load(light_shaders, light_vert_source,
                            light_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "light shader compilation error\n");
        goto cleanup_gl;
    }
    light_shaders->use(light_shaders);
    vec3 light_color = {2.0f, 0.7f, 1.3f};
    light_shaders->setVec3(light_shaders, "light_color", light_color);

    struct Shader * cube_shaders = shaderInit();
    if (cube_shaders->load(cube_shaders, cube_vert_source,
                           cube_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "cube shader compilation error\n");
        goto cleanup_gl;
    }
    cube_shaders->use(cube_shaders);
    cube_shaders->setVec3(cube_shaders, "light_color", light_color);
    cube_shaders->setInt(cube_shaders, "material.diffuse", 0);
    vec3 cube_specular = {0.5f, 0.5f, 0.5f};
    vec3 light_ambient = {0.2f, 0.2f, 0.2f};
    vec3 light_diffuse = {0.5f, 0.5f, 0.5f};
    vec3 light_specular = {1.f, 1.f, 1.f};
    float shininess = 32.0f;
    cube_shaders->setVec3(cube_shaders, "material.specular", cube_specular);
    cube_shaders->setFloat(cube_shaders, "material.shininess", shininess);
    cube_shaders->setVec3(cube_shaders, "light.ambient", light_ambient);
    cube_shaders->setVec3(cube_shaders, "light.diffuse", light_diffuse);
    cube_shaders->setVec3(cube_shaders, "light.specular", light_specular);

    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "%s %d: GL Error %x\n", __LINE__, __FILE__, glError);
        glfwTerminate();
        exit(1);
    }

    /* shader introspection */
    /*
    light_shaders->use(light_shaders);
    printf("Shader introspection for light:\n");
    shader_introspection(light_shaders);
    cube_shaders->use(cube_shaders);
    printf("Shader introspection for cube:\n");
    shader_introspection(cube_shaders);
    */

    glBindVertexArray(cube_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);
    glEnable(GL_DEPTH_TEST);

    int numFrames = 0;
    float past = (float)glfwGetTime();
    mat4x4 model;
    /* This is the one that transforms the normals correctly,
     * even if there is a scaling present in the model matrix.
     */
    mat4x4 normal_matrix;

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;
        float time = (float)glfwGetTime();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwCompatKeyboardCallback(window);

        // Update colors
        light_color[0] = sin(2.f*time);
        light_color[1] = sin(0.7f*time);
        light_color[2] = sin(1.3f*time);
        light_diffuse[0] = sin(0.5f*2.f*time);
        light_diffuse[1] = sin(0.5f*0.7f*time);
        light_diffuse[2] = sin(0.5f*1.3f*time);
        light_ambient[0] = sin(0.2f*2.f*time);
        light_ambient[1] = sin(0.2f*0.7f*time);
        light_ambient[2] = sin(0.2f*1.3f*time);

        // draw the light
        light_shaders->use(light_shaders);
        light_shaders->setVec3(light_shaders, "light_color", light_color);
        glBindVertexArray(light_VAO);
        cam->setViewMatrix(cam, light_shaders, "view");
        cam->setProjectionMatrix(cam, light_shaders, "projection");
        mat4x4_translate(model, light_position[0],
                         light_position[1], light_position[2]);
        sendMatrixToShader(model, "model", light_shaders);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw cubes
        cube_shaders->use(cube_shaders);
        glBindVertexArray(cube_VAO);
        cube_shaders->setVec3(cube_shaders, "light.position", light_position);
        cube_shaders->setVec3(cube_shaders, "camera_position", *cam->position);
        cam->setViewMatrix(cam, cube_shaders, "view");
        cam->setProjectionMatrix(cam, cube_shaders, "projection");
        cube_shaders->setVec3(cube_shaders, "light.ambient", light_ambient);
        cube_shaders->setVec3(cube_shaders, "light.diffuse", light_diffuse);
        cube_shaders->setVec3(cube_shaders, "light.color", light_color);
        
        for (int i=0; i<10; i++){
            float x, y, z;
            x = cubePositions[3*i];
            y = cubePositions[3*i+1];
            z = cubePositions[3*i+2];
            mat4x4_translate(model, x, y, z);
            if (i%3 == 0){
                float angle = time*50*M_PI/180;
                float offset = 20*i;
                mat4x4_rotate(model, model, 0.5, 1, 0, angle+offset);
            }
            else{
                mat4x4_rotate(model, model, 0.5, 1, 0, 0);
            }
            sendMatrixToShader(model, "model", cube_shaders);
            mat4x4_normal_matrix(normal_matrix, model);
            sendMatrixToShader(normal_matrix, "normal_matrix", cube_shaders);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    float time = (float)glfwGetTime();
    printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n",
           numFrames, time, numFrames/time);

    cleanup_gl:
        glDeleteVertexArrays(1, &light_VAO);
        glDeleteBuffers(1, &light_VBO);
        glDeleteVertexArrays(1, &cube_VAO);
        glDeleteBuffers(1, &cube_VBO);
    cleanup_glfw:
        glfwTerminate();
    end:
        return status;
}
