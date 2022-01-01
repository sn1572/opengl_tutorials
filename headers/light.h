#ifndef _LIGHT_HEADER
    #define _LIGHT_HEADER

    #include <stdio.h>
    #include <glad/glad.h>
    #include <linmath.h>
    #include <shader.h>

    typedef enum {
        LIGHT_SUCCESS =  0,
        LIGHT_ERR     = -1,
    } light_error_t;

    #ifndef err_print
        #define err_print(msg){\
            fprintf(stderr, "%s %d: "msg"\n", __FILE__, __LINE__);\
        }
    #endif

    struct Light{
        char * name;
        vec3 position;              //not for directional
        vec3 direction;             //not for point
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
        float theta_min;            //spotlight
        float theta_taper_start;    //spotlight
        float constant;             //point light
        float linear;               //point light
        float quadratic;            //point light
        unsigned int depth_FBO;     //shadows
        unsigned int depth_texture; //shadows
        unsigned int shadow_width;  //shadows
        unsigned int shadow_height; //shadows
        mat4x4 shadow_matrix;       //shadows
    };
    typedef struct Light Light;

    light_error_t light_init(Light * light);
    light_error_t light_to_shader(Light * light, struct Shader * shader);
    light_error_t light_shadow_gl_init(Light * light);
    light_error_t light_shadow_mat_directional(Light * light, vec3 center,
                                               vec3 up, float near_plane,
                                               float far_plane,
                                               vec4 ortho_params);

#endif
