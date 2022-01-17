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


#ifdef DRAW_DEPTH_MAP
static char model_frag_source[] = "shaders/depth.frag";
#else
static char model_frag_source[] = "shaders/model.frag";
#endif
static char model_vert_source[] = "shaders/model.vert";
static char depth_frag_source[] = "shaders/point_shadow.frag";
static char depth_geom_source[] = "shaders/point_shadow.geom";
static char depth_vert_source[] = "shaders/point_shadow.vert";
static char texture_frag_source[] = "shaders/loading_shader.frag";
static char texture_vert_source[] = "shaders/loading_shader.vert";
static char skybox_vert_source[] = "shaders/skybox.vert";
static char skybox_frag_source[] = "shaders/skybox.frag";
static int WIDTH = 1920;
static int HEIGHT = 1080;


const float quad_data[] = {
    //positions           texture coordinates
	 1.f,  1.f,  0.0f,  1.0f,  1.0f,
	 1.f, -1.f,  0.0f,  1.0f,  0.0f,
	-1.f, -1.f,  0.0f,  0.0f,  0.0f,
    -1.f, -1.f,  0.f,   0.0f,  0.0f,
    -1.f,  1.f,  0.f,   0.0f,  1.0f,
	 1.f,  1.f,  0.0f,  1.0f,  1.0f
};


const float cube_data[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
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


int cubemap_load(unsigned int * id, char ** files)
{
    int width, height, nrChannels;
    unsigned char * data;
    GLenum format;

    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *id);
    for(int i = 0; i < 6; i++){
        data = stbi_load(files[i], &width, &height, &nrChannels, 0);
        if (data){
            switch (nrChannels){
                case 1:
                    format = GL_RED;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
                default:
                    err_print("unrecognized texture format");
                    break;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width,
                         height, 0, format, GL_UNSIGNED_BYTE, data);
        } else{
            fprintf(stderr, "%s %d: Error loading file %i\n", __FILE__,
                    __LINE__, i);
            return -1;
        }
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return 0;
}


int main(){
    int status = SUCCESS;
    int numFrames = 0;
    mat4x4 model_matrix;
    mat4x4 normal_matrix;
    float time;
    char model_path[] = "../../model_loading/model/models/backpack/"
                        "backpack.obj";
    const char * skybox_paths[6];
    skybox_paths[0] = "../../model_loading/model/models/skybox/right.jpg";
    skybox_paths[1] = "../../model_loading/model/models/skybox/left.jpg";
    skybox_paths[2] = "../../model_loading/model/models/skybox/top.jpg";
    skybox_paths[3] = "../../model_loading/model/models/skybox/bottom.jpg";
    skybox_paths[4] = "../../model_loading/model/models/skybox/front.jpg";
    skybox_paths[5] = "../../model_loading/model/models/skybox/back.jpg";

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
    if (shaderLoad(depth_shader, depth_vert_source,
                   depth_frag_source, depth_geom_source) != SHADER_NO_ERR){
        err_print("point shadow shader compile error");
        goto cleanup_gl;
    }
    struct Shader * loading_shader = shaderInit();
    if (load(loading_shader, texture_vert_source,
             texture_frag_source) != SHADER_NO_ERR){
        err_print("loading screen shader compile error");
        goto cleanup_gl;
    }
    struct Shader * skybox_shader = shaderInit();
    if (load(skybox_shader, skybox_vert_source,
             skybox_frag_source) != SHADER_NO_ERR){
        err_print("skybox shader compile error");
        goto cleanup_gl;
    }

    // Loading screen
    draw_loading_screen(window, plane_vao, loading_shader);

    // An untextured cube
    unsigned int cube_vao;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);
    unsigned int cube_vbo;
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_data), cube_data,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glBindVertexArray(0);

    // A nontrivial model
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
    light_shadow_cube_map_init(&light);
    light.name = malloc(12 * sizeof(char));
    snprintf(light.name, 12, "point_light");
    vec3 point_ambient = {0.4f, 0.4f, 0.4f};
    vec3 point_diffuse = {1.f, 1.f, 1.f};
    vec3 point_specular = {3.f, 3.f, 3.f};
    vec3_dup(light.ambient, point_ambient);
    vec3_dup(light.diffuse, point_diffuse);
    vec3_dup(light.specular, point_specular);

    /* Cubemap setup */
    unsigned int skybox_id;
    if (cubemap_load(&skybox_id, (char **)skybox_paths)){
        err_print("error during skybox load");
        exit(1);
    }

    /* Almost forgot this is anti-aliased */
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    float glfw_loop_start_time = (float)glfwGetTime();

    while (!glfwWindowShouldClose(window)){
        numFrames += 1;
        time = (float)glfwGetTime();

        glClearColor(0.2f, 0.2f, 0.2f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        float far_plane = 10.f;
        light_shadow_cube_mat(&light, 1.f, far_plane);
        glBindFramebuffer(GL_FRAMEBUFFER, light.depth_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        use(depth_shader);
        light_to_shader(&light, depth_shader);
        setFloat(depth_shader, "far_plane", far_plane);
        setMat4x4(depth_shader, "model_matrix", model_matrix);
        glViewport(0, 0, light.shadow_width, light.shadow_height);
        GL_ERR_CHECK;
        model_error_t draw_result;
        if (draw_result = draw_model(depth_shader, backpack)){
            err_print("failure drawing with depth shader");
            fprintf(stderr, "Error code: %x\n", draw_result);
            goto cleanup_gl;
        }

        /* Undo shadow configuration */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, WIDTH, HEIGHT);
        glCullFace(GL_BACK);

        /* Draw model */
        use(model_shader);
        setViewMatrix(cam, model_shader, "view");
        setProjectionMatrix(cam, model_shader, "projection");
        setMat4x4(model_shader, "model_matrix", model_matrix);
        setMat4x4(model_shader, "normal_matrix", normal_matrix);
        setVec3(model_shader, "camera_position", *cam->position);
        int max_texture_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
        int texture_unit = cached_texture_count(backpack);
        if (texture_unit < max_texture_units){
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_CUBE_MAP, light.depth_texture);

        } else{
            err_print("Not enough texture units for point light cube map");
        }
        if (texture_unit+1 < max_texture_units){
            glActiveTexture(GL_TEXTURE0 + texture_unit + 1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_id);
            setInt(model_shader, "skybox", texture_unit+1);
        } else{
            err_print("Not enough texture units for skybox");
        }
        /* Note: This is a bit confusing.
         * On the host side the light struct has just one texture entry
         * called "depth_texture." That is sometimes a regular 2D texture
         * and sometimes a cube map.
         * In the shader these two texture types are distinct, so the shader
         * version of the light struct has *both* a sampler2D called
         * depth_texture and a samplerCube called cube_map.
         * The design of the light.c source is such that you either
         * initialize the light as a directional light or as a point
         * light (the *_cube_map_init version) and will return some error
         * codes if you try to intialize both ways.
         * It might be wise to introduce a type describing what kind of light
         * is contained in the struct to make sure that everything is very
         * explicitly connected to what type of light is being used
         * (directional vs. point_light).
         */
        setInt(model_shader, "point_light.cube_map", texture_unit);
        setFloat(model_shader, "material.shininess", 4.f);
        setFloat(model_shader, "far_plane", far_plane);
        light_to_shader(&light, model_shader);
        draw_model(model_shader, backpack);

        /* skybox */
        glDepthFunc(GL_LEQUAL);
        //glDepthMask(GL_FALSE);
        use(skybox_shader);
        glBindVertexArray(cube_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_id);
        setInt(skybox_shader, "skybox", 0);
        setViewMatrix(cam, skybox_shader, "view");
        setProjectionMatrix(cam, skybox_shader, "projection");
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

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
