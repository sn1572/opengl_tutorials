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
#include <light.h>


#define SUCCESS 0;
#define FAILURE 1;


static char model_frag_source[] = "shaders/model.frag";
static char model_vert_source[] = "shaders/model.vert";
static char depth_frag_source[] = "shaders/depth.frag";
static char depth_vert_source[] = "shaders/depth.vert";
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
    char model_path[] = "../../model_loading/model/models/backpack/"
                        "backpack.obj";

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
    }
    setup_model(&backpack);

    Texture_Node * node;
    int num_textures = 0;
    for (node = backpack.loaded_textures; node; node = node->next){
        num_textures += 1;
    }
    printf("Backpack texture count: %i\n", num_textures);

    struct Shader * model_shader = shaderInit();
    if (load(model_shader, model_vert_source,
             model_frag_source) != SHADER_NO_ERR){
        err_print("model shader compile error");
        goto cleanup_gl;
    }
    struct Shader * depth_shader = shaderInit();
    if (load(model_shader, depth_vert_source,
             depth_frag_source) != SHADER_NO_ERR){
        err_print("depth shader compile error");
        goto cleanup_gl;
    }

    struct Camera * cam;
    cam = cameraInit(WIDTH, HEIGHT);
    cam->movementSpeed = 5.f;
    setActiveCamera(cam);
    setActiveCameraPosition(0, 0, 3);

    Light light;
    light_init(&light);
    /* This allocates a framebuffer, texture, etc. */
    light_shadow_gl_init(&light);
    light.name = malloc(12 * sizeof(char));
    snprintf(light.name, 12, "point_light");
    vec3 point_ambient = {0.4f, 0.4f, 0.4f};
    vec3 point_diffuse = {1.f, 1.f, 1.f};
    vec3 point_specular = {3.f, 3.f, 3.f};
    vec3_dup(light.ambient, point_ambient);
    vec3_dup(light.diffuse, point_diffuse);
    vec3_dup(light.specular, point_specular);
    printf("Light name: %s\n", light.name);

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

        vec3 center = {0.f, 0.f, 0.f};
        vec3 up = {0.f, 1.f, 0.f};
        vec4 ortho_params = {-10.f, 10.f, -10.f, 10.f};
        light_shadow_mat_directional(&light, center, up, 1.f, 7.5f,
                                     ortho_params);
        use(depth_shader);
        light_to_shader(&light, depth_shader);
        glViewport(0, 0, light.shadow_width, light.shadow_height);
        glBindFramebuffer(GL_FRAMEBUFFER, light.depth_FBO);
        /* Need to make sure the depth buffer is clear before calling
         * draw methods with the depth shader. We cleared it above.
         */
        draw_model(depth_shader, backpack);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WIDTH, HEIGHT);
        /* There is no color buffer in use during the depth rendering,
         * so we only need to clear the depth buffer.
         */
        glClear(GL_DEPTH_BUFFER_BIT);
        use(model_shader);
        setViewMatrix(cam, model_shader, "view");
        setProjectionMatrix(cam, model_shader, "projection");
        mat4x4_identity(model_matrix);
        mat4x4_identity(normal_matrix);
        sendMatrixToShader(model_matrix, "model_matrix", model_shader);
        sendMatrixToShader(normal_matrix, "normal_matrix", model_shader);

        /* apply point light effects */
        vec4 light_position_4;
        vec3 light_position;
        mat4x4 R;
        mat4x4_identity(R);
        float angle = time*50*M_PI/180;
        mat4x4_rotate(R, R, 0.f, 1.f, 0.f, angle);
        vec4 light_initial_position = {4.f, 0.f, 0.f, 0.f};
        mat4x4_mul_vec4(light_position_4, R, light_initial_position);
        vec3_dup(light.position, light_position_4);
        setFloat(model_shader, "material.shininess", 4.f);
        light_to_shader(&light, model_shader);
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
