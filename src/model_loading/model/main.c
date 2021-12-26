#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <shader.h>
#include <stdbool.h>
#include <math.h>
#include <linmath.h>
#include <linmath_extension.h>
#include <camera.h>
#include <shader.h>
#include <model.h>


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
    int numFrames = 0;
    mat4x4 model;
    mat4x4 normal_matrix;
    float past, time;
    char model_path[] = "./models/backpack/backpack.obj";

    Model backpack;
    backpack.file_path = model_path;
    backpack.meshes = NULL;
    backpack.directory = NULL;
    backpack.num_meshes = 0;
    if (!load_model(backpack)){
        fprintf(stderr, "%s %d: Failed to load backpack model.\n", __FILE__,
                __LINE__);
        goto end;
    }
    printf("Loaded backpack model, as far as I can tell.\n");
    goto cleanup_glfw;

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

    past = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;
        time = (float)glfwGetTime();

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwCompatKeyboardCallback(window);

        /*
        cam->setViewMatrix(cam, cube_shaders, "view");
        cam->setProjectionMatrix(cam, cube_shaders, "projection");
        */

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    time = (float)glfwGetTime();
    printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n",
           numFrames, time, numFrames/time);

    //cleanup_gl:
        //free_model(backpack);
    cleanup_glfw:
        glfwTerminate();
    end:
        return status;
}
