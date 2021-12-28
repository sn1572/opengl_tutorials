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


static char model_frag_source[] = "shaders/model_frag";
static char model_vert_source[] = "shaders/model_vert";
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


int main(){
    int status = SUCCESS;
    int numFrames = 0;
    mat4x4 model_matrix;
    mat4x4 normal_matrix;
    float past, time;
    char model_path[] = "./models/backpack/backpack.obj";

    /* glfw init and context creation */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpengl", NULL,
                                          NULL);
    if (window == NULL){
        fprintf(stderr, "Failed to create a GLFW window.\n");
        status = FAILURE;
        goto cleanup_glfw;
    }
    glfwMakeContextCurrent(window);
    /* user input callbacks */
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, glfwCompatMouseMovementCallback);
    glfwSetScrollCallback(window, glfwCompatMouseScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //initialize GLAD loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        fprintf(stderr, "Failed to initialize GLAD\n");
        status = FAILURE;
        goto cleanup_glfw;
    }

    // set default window size
    glViewport(0, 0, WIDTH, HEIGHT);

    /* model loading
     * Eventually this will need to have its own thread to prevent
     * application non-responsiveness. To do that we probably need to
     * pull out the opengl calls.
     */

    Model backpack;
    backpack.file_path = model_path;
    backpack.meshes = NULL;
    backpack.directory = NULL;
    backpack.num_meshes = 0;
    backpack.loaded_textures = NULL;
    if (load_model(&backpack)){
        fprintf(stderr, "%s %d: Failed to load backpack model.\n", __FILE__,
                __LINE__);
        goto end;
    } else{
        printf("Backpack loaded successfully (as far as I can tell).\n");
    }
    setup_model(&backpack);

    /* model inspection */
    /*
    Texture_Node * node;
    for (node = backpack.loaded_textures; node; node = node->next){
        printf("Loaded texture: %s\n", (node->texture).path);
    }
    for (int mesh_no = 0; mesh_no < backpack.num_meshes; mesh_no++){
        printf("Vertices for mesh %i\n", mesh_no);
        for (int t = 0; t < backpack.meshes[mesh_no].num_vertices/100; t++){
            printf("\tMesh %i normal %i: %4.2f, %4.2f, %4.2f\n", mesh_no, t*100,
                   backpack.meshes[mesh_no].vertices[t*100].normal[0],
                   backpack.meshes[mesh_no].vertices[t*100].normal[1],
                   backpack.meshes[mesh_no].vertices[t*100].normal[2]);
        }
    }
    */
    /*
    for (int t = 0; t < backpack.num_meshes; t++){
        printf("Mesh %i: %u vertices, %u indices, %u textures\n", t,
               backpack.meshes[t].num_vertices,
               backpack.meshes[t].num_indices,
               backpack.meshes[t].num_textures);
        printf("Texture ids: %u, %u\n", backpack.meshes[t].textures[0].id,
               backpack.meshes[t].textures[1].id);
    }
    */

    /* Shader init */
    struct Shader * light_shader = shaderInit();
    if (light_shader->load(light_shader, light_vert_source,
                           light_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "light shader compilation error\n");
        goto cleanup_gl;
    }
    struct Shader * model_shader = shaderInit();
    if (load(model_shader, model_vert_source,
             model_frag_source) != SHADER_NO_ERR){
        fprintf(stderr, "model shader compilation error\n");
        goto cleanup_gl;
    }

    // Init the camera `object` and hook its methods into the callbacks
    struct Camera * cam;
    cam = cameraInit(WIDTH, HEIGHT);
    cam->movementSpeed = 5.f;
    setActiveCamera(cam);
    setActiveCameraPosition(0, 0, 3);

    /* callback assignment former location */

    /* main loop */
    past = (float)glfwGetTime();
    /* Turn on depth testing before drawing anything */
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;
        time = (float)glfwGetTime();

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Can this be moved outside the main loop?
        glfwCompatKeyboardCallback(window);

        use(model_shader);
        setViewMatrix(cam, model_shader, "view");
        setProjectionMatrix(cam, model_shader, "projection");
        mat4x4_identity(model_matrix);
        //mat4x4_normal_matrix(normal_matrix, model_matrix);
        mat4x4_identity(normal_matrix);
        sendMatrixToShader(model_matrix, "model_matrix", model_shader);
        sendMatrixToShader(normal_matrix, "normal_matrix", model_shader);

        /* apply point light effects */
        vec3 point_ambient = {0.4f, 0.4f, 0.4f};
        vec3 point_diffuse = {2.f, 2.f, 2.f};
        vec3 point_specular = {5.f, 5.f, 5.f};
        vec3 point_position = {2.f, 2.f, 2.f};
        setVec3(model_shader, "point_light.position", point_position);
        setVec3(model_shader, "point_light.ambient", point_ambient);
        setVec3(model_shader, "point_light.specular", point_specular);
        setVec3(model_shader, "point_light.diffuse", point_diffuse);
        setFloat(model_shader, "point_light.constant", 1.f);
        setFloat(model_shader, "point_light.linear", 0.07f);
        setFloat(model_shader, "point_light.quadratic", 0.017f);
        setFloat(model_shader, "material.shininess", 32.f);

        draw_model(model_shader, backpack);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    time = (float)glfwGetTime();
    printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n",
           numFrames, time, numFrames/time);

    cleanup_gl:
        free_model(&backpack);
    cleanup_glfw:
        glfwTerminate();
    end:
        return status;
}
