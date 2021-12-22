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
static char light_frag_source[] = "shaders/light_frag";
static char light_vert_source[] = "shaders/light_vert";
static int WIDTH = 1920;
static int HEIGHT = 1080;


void print_array(float * array, int rows, int cols){
    for (int i=0; i<rows; i++){
        for (int j=0; j<cols; j++){
            printf("%4.2f ", array[i*cols+j]);
        }
        printf("\n");
    }
}


int image_to_texture(const char * file_name){
    int width, height, nrChannels;
    unsigned char * data = stbi_load(file_name, &width, &height,
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
                stbi_image_free(data);
                return 1;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return 0;
    }
    else{
        fprintf(stderr, "%f %d: Failed to load texture\n", __FILE__,
                __LINE__);
        return 1;
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


vec3 light_positions[] = {
    {0.7f, 0.2f, 2.0f},
    {2.3f, -3.3f, -4.0f},
    {-4.0f, 2.0f, -12.0f},
    {0.f, 0.f, -3.f}
};


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
    glfwSetCursorPosCallback(window, glfwCompatMouseMovementCallback);
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
    
    //Cube texture (aka diffuse map)
    unsigned int diffuse_map;
    glGenTextures(1, &diffuse_map);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (image_to_texture("models/container2.png")){
        status = FAILURE;
        goto cleanup_gl;
    }

    //Cube specular map
    unsigned int specular_map;
    glGenTextures(1, &specular_map);
    glBindTexture(GL_TEXTURE_2D, specular_map);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (image_to_texture("models/container2_specular.png")){
        status = FAILURE;
        goto cleanup_gl;
    }

    struct Shader * light_shaders = shaderInit();
    if (light_shaders->load(light_shaders, light_vert_source,
                            light_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "light shader compilation error\n");
        goto cleanup_gl;
    }
    struct Shader * cube_shaders = shaderInit();
    if (cube_shaders->load(cube_shaders, cube_vert_source,
                           cube_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "cube shader compilation error\n");
        goto cleanup_gl;
    }

    /* This is the moment that textures get bound to texture units.
     * It is these texture unit ids (0 and 1 here) that get referenced
     * by the shaders.
     * I got confused by the texture IDs (eg. diffuse_map is 1 and
     * specular_map is 2 when printf'ing them). You can generate in principal
     * a large number of textures, but can only bind a few to texture units
     * at a time (the minimun guaranteed in GL 4.x is 16 per stage with 6
     * stages, including compute shaders).
     */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_map);
    glEnable(GL_DEPTH_TEST);

    use(cube_shaders);
    setInt(cube_shaders, "material.diffuse", 0);
    setInt(cube_shaders, "material.specular", 1);
    setFloat(cube_shaders, "material.shininess", 32.f);
    // point lights
    vec3 point_ambient = {0.4f, 0.4f, 0.4f};
    vec3 point_diffuse = {1.f, 1.f, 1.f};
    vec3 point_specular = {3.f, 3.f, 3.f};
    for (int light_index=0; light_index<4; light_index++){
        char name[25];
        snprintf(name, 20, "point_lights[%i].position", light_index);
        setVec3(cube_shaders, name, light_positions[light_index]);
        snprintf(name, 20, "point_lights[%i].ambient", light_index);
        setVec3(cube_shaders, name, point_ambient);
        snprintf(name, 20, "point_lights[%i].diffuse", light_index);
        setVec3(cube_shaders, name, point_diffuse);
        snprintf(name, 20, "point_lights[%i].specular", light_index);
        setVec3(cube_shaders, name, point_specular);
        snprintf(name, 20, "point_lights[%i].constant", light_index);
        setFloat(cube_shaders, name, 1.f);
        snprintf(name, 20, "point_lights[%i].linear", light_index);
        setFloat(cube_shaders, name, 0.35f);
        snprintf(name, 20, "point_lights[%i].quadratic", light_index);
        setFloat(cube_shaders, name, 0.44f);
    }
    // directional light
    vec3 directional_ambient = {0.2f, 0.2f, 0.1f};
    vec3 directional_diffuse = {0.3f, 0.3f, 0.2f};
    vec3 directional_specular = {0.5f, 0.5f, 0.3f};
    vec3 directional_direction = {-0.2f, -1.f, -0.3f};
    setVec3(cube_shaders, "directional_light.ambient", directional_ambient);
    setVec3(cube_shaders, "directional_light.diffuse", directional_diffuse);
    setVec3(cube_shaders, "directional_light.specular", directional_specular);
    setVec3(cube_shaders, "directional_light.direction", directional_direction);
    // spotlight (update position and direction in main loop)
    vec3 spotlight_ambient = {0.4f, 0.4f, 0.4f};
    vec3 spotlight_diffuse = {3.f, 3.f, 3.f};
    vec3 spotlight_specular = {10.f, 10.f, 10.f};
    setVec3(cube_shaders, "spotlight.ambient", spotlight_ambient);
    setVec3(cube_shaders, "spotlight.diffuse", spotlight_diffuse);
    setVec3(cube_shaders, "spotlight.specular", spotlight_specular);
    setFloat(cube_shaders, "spotlight.theta_taper_start", 0.9f);
    setFloat(cube_shaders, "spotlight.theta_min", 0.8f);

    light_shaders->use(light_shaders);
    light_shaders->setVec3(light_shaders, "light.ambient", spotlight_diffuse);

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

        // draw the lights
        light_shaders->use(light_shaders);
        glBindVertexArray(light_VAO);
        cam->setViewMatrix(cam, light_shaders, "view");
        cam->setProjectionMatrix(cam, light_shaders, "projection");
        for (int i=0; i<4; i++){
            mat4x4_translate(model, light_positions[i][0],
                             light_positions[i][1], light_positions[i][2]);
            mat4x4_scale(model, model, 0.1f);
            sendMatrixToShader(model, "model", light_shaders);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // draw cubes
        cube_shaders->use(cube_shaders);
        glBindVertexArray(cube_VAO);
        cube_shaders->setVec3(cube_shaders, "camera_position", *cam->position);
        setVec3(cube_shaders, "spotlight.position", *cam->position);
        setVec3(cube_shaders, "spotlight.direction", *cam->front);
        printf("cam pos: %4.2f %4.2f %4.2f\n", *cam->position[0],
               *cam->position[1], *cam->position[2]);
        printf("cam front: %4.2f %4.2f %4.2f\n", *cam->front[0],
               *cam->front[1], *cam->front[2]);
        cam->setViewMatrix(cam, cube_shaders, "view");
        cam->setProjectionMatrix(cam, cube_shaders, "projection");
        
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
