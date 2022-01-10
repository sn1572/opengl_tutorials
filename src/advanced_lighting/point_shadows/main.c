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

#define GL_ERR_CHECK do{\
    GLenum glError = glGetError();\
    if (glError != GL_NO_ERROR){\
        fprintf(stderr, "%s %d: GL error %x\n", __FILE__, __LINE__,\
                glError);\
    }\
} while (0)


static char model_frag_source[] = "shaders/shadow.frag";
static char model_vert_source[] = "shaders/shadow.vert";
static char depth_frag_source[] = "shaders/depth.frag";
static char depth_vert_source[] = "shaders/depth.vert";
static char texture_frag_source[] = "shaders/texture_render.frag";
static char texture_vert_source[] = "shaders/texture_render.vert";
static int WIDTH = 1920;
static int HEIGHT = 1080;


static const float quad_data[] = {
    //positions           texture coordinates
	 1.f,  1.f,  0.0f,  1.0f,  1.0f,
	 1.f, -1.f,  0.0f,  1.0f,  0.0f,
	-1.f, -1.f,  0.0f,  0.0f,  0.0f,
    -1.f, -1.f,  0.f,   0.0f,  0.0f,
    -1.f,  1.f,  0.f,   0.0f,  1.0f,
	 1.f,  1.f,  0.0f,  1.0f,  1.0f
};


void print_array(float * array, int rows, int cols){
    for (int i=0; i<rows; i++){
        for (int j=0; j<cols; j++){
            printf("%4.2f ", array[i*cols+j]);
        }
        printf("\n");
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void basic_loading_screen(GLFWwindow * window)
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
}


void draw_loading_screen(GLFWwindow * window, unsigned int plane_vao,
                         struct Shader * texture_shader)
{
    /* Be sure texture shader is available */
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned int loading_screen_id;
    // This handy function comes from model.c. Hooray abstraction :)
    if (texture_from_file("../../model_loading/model/models/"
                          "loading_screen.jpg", &loading_screen_id)){
        err_print("error loading loading screen texture (l0l)");
    }

    shader_err_t result;
    GL_ERR_CHECK;
    result = use(texture_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, loading_screen_id);
    setInt(texture_shader, "texture_to_render", 0);
    glBindVertexArray(plane_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(window);
}


int main(){
    int status = SUCCESS;
    int numFrames = 0;
    mat4x4 model_matrix;
    mat4x4 normal_matrix;
    float time;
    char model_path[] = "../../model_loading/model/models/backpack/"
                        "backpack.obj";
    const int AA_RATE = 4;
    clock_t clock_start, diff;

    /* glfw init and context creation */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, AA_RATE);
    // According to the docs this should get us the highest available rate.
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Trump did 9/11",
                                          glfwGetPrimaryMonitor(), NULL);
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

    // For rendering textures directly to the screen
    unsigned int plane_vao;
    glGenVertexArrays(1, &plane_vao);
    glBindVertexArray(plane_vao);
    unsigned int plane_vbo;
    glGenBuffers(1, &plane_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glBindVertexArray(0);

    struct Shader * model_shader = shaderInit();
    if (load(model_shader, model_vert_source,
             model_frag_source) != SHADER_NO_ERR){
        err_print("model shader compile error");
        goto cleanup_gl;
    }
    struct Shader * depth_shader = shaderInit();
    if (load(depth_shader, depth_vert_source,
             depth_frag_source) != SHADER_NO_ERR){
        err_print("depth shader compile error");
        goto cleanup_gl;
    }
    struct Shader * texture_render = shaderInit();
    if (load(texture_render, texture_vert_source,
             texture_frag_source) != SHADER_NO_ERR){
        err_print("texture render shader compile error");
        goto cleanup_gl;
    }

    // Loading screen
    draw_loading_screen(window, plane_vao, texture_render);

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

    struct Camera * cam;
    cam = cameraInit(WIDTH, HEIGHT);
    cam->movementSpeed = 5.f;
    setActiveCamera(cam);
    setActiveCameraPosition(0, 0, 3);

    Light light;
    light_init(&light);
    /* Set the shadow texture resolution before the gl init call. 
     * Don't change it after that.
     */
    light.shadow_width = 2048;
    light.shadow_height = 2048;
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

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    float glfw_loop_start_time = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;
        time = (float)glfwGetTime();

        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Can this be moved outside the main loop?
        glfwCompatKeyboardCallback(window);

        /* Parameters shared by all shaders */
        vec4 light_position_4;
        vec3 light_position;
        vec4 light_initial_position = {4.f, 0.f, 0.f, 0.f};
        mat4x4 R;
        mat4x4_identity(R);
        float angle = time*50*M_PI/180;
        mat4x4_rotate(R, R, 0.f, 1.f, 0.f, angle);
        mat4x4_mul_vec4(light_position_4, R, light_initial_position);
        vec3_dup(light.position, light_position_4);
        mat4x4_identity(model_matrix);
        mat4x4_identity(normal_matrix);

        /* depth mapping */
        /* Apparently this is the point the camera is rotating around. */
        vec3 center = {0.f, 0.f, -1.f};
        vec3 up = {0.f, 1.f, 0.f};
        /* The larger this is, the smaller the model appears.
         * In other words turn this down to increase shadow resolution
         * on the backpack. Bear in mind the cost is that the scope
         * of the scene captured is reduced.
         */
        float ortho_mag = 4.f;
        vec4 ortho_params = {-1.f, 1.f, -1.f, 1.f};
        vec4_scale(ortho_params, ortho_params, ortho_mag);
        light_shadow_mat_directional(&light, center, up, 1.f, 7.5f,
                                     ortho_params);
        glBindFramebuffer(GL_FRAMEBUFFER, light.depth_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        use(depth_shader);
        setMat4x4(depth_shader, "light_space_matrix", light.shadow_matrix);
        setMat4x4(depth_shader, "model_matrix", model_matrix);
        glViewport(0, 0, light.shadow_width, light.shadow_height);
        GL_ERR_CHECK;
        model_error_t draw_result;
        if (draw_result = draw_model(depth_shader, backpack)){
            err_print("failure drawing with depth shader");
            fprintf(stderr, "Error code: %x\n", draw_result);
            goto cleanup_gl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        /* If window resizeable, need to save and re-use current width
         * and height.
         */
        glViewport(0, 0, WIDTH, HEIGHT);
        glCullFace(GL_BACK);

        #ifdef DRAW_DEPTH_MAP
        shader_err_t result;
        GL_ERR_CHECK;
        result = use(texture_render);
        if (result != SHADER_NO_ERR){
            err_print("Error using texture_render");
            goto cleanup_gl;
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, light.depth_texture);
        //glBindTexture(GL_TEXTURE_2D, backpack.loaded_textures->texture.id);
        setInt(texture_render, "texture_to_render", 0);
        glBindVertexArray(plane_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        #else
        use(model_shader);
        setViewMatrix(cam, model_shader, "view");
        setProjectionMatrix(cam, model_shader, "projection");
        setMat4x4(model_shader, "model_matrix", model_matrix);
        setMat4x4(model_shader, "normal_matrix", normal_matrix);
        int max_texture_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
        int texture_unit = cached_texture_count(backpack);
        if (texture_unit < max_texture_units){
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, light.depth_texture);
            setInt(model_shader, "point_light.depth_texture", texture_unit);
        } else{
            err_print("Not enough texture units. omg");
        }

        setFloat(model_shader, "material.shininess", 4.f);
        light_to_shader(&light, model_shader);
        draw_model(model_shader, backpack);
        #endif

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glGetError() != GL_NO_ERROR){
            glfwSetWindowShouldClose(window, 1);
            err_print("GL error detected. Bailing out.");
        }
    }
    time = (float)glfwGetTime() - glfw_loop_start_time;
    printf("Rendered %i frames in %1.10f seconds amounting to %f FPS.\n",
           numFrames, time, numFrames / time);

    cleanup_gl:
        free_model(&backpack);
    cleanup_glfw:
        glfwTerminate();
    end:
        return status;
}
