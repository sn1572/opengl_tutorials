#include <light.h>


light_error_t light_init(Light * light)
{
    mat4x4 zeros = {{0.f, 0.f, 0.f, 0.f},
                    {0.f, 0.f, 0.f, 0.f},
                    {0.f, 0.f, 0.f, 0.f},
                    {0.f, 0.f, 0.f, 0.f}};
    vec3 zero_vec = {0.f, 0.f, 0.f};
    if (!light){
        err_print("Attempting to initialize unallocated pointer");
        return LIGHT_ERR;
    }
    vec3_dup(light->position,  zero_vec);
    vec3_dup(light->direction, zero_vec);
    vec3_dup(light->ambient,   zero_vec);
    vec3_dup(light->diffuse,   zero_vec);
    vec3_dup(light->specular,  zero_vec);
    light->name              = NULL;
    light->theta_min         = 0.5f;
    light->theta_taper_start = 0.6f;
    light->constant          = 1.0f;
    light->linear            = 0.07f;
    light->quadratic         = 0.017f;
    light->depth_FBO         = 0;
    light->depth_texture     = 0;
    light->shadow_width      = 1024;
    light->shadow_height     = 1024;
    mat4x4_dup(light->shadow_matrix, zeros);
    light->cube_mats         = NULL;
    return LIGHT_SUCCESS;
}


light_error_t light_to_shader(Light * light, struct Shader * shader)
{
    int string_length = strlen(light->name);
    int max_unif_name = string_length + 20;
    char uniform_name[max_unif_name];
    int retval;

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.position", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setVec3(shader, uniform_name, light->position);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.direction", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setVec3(shader, uniform_name, light->direction);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.ambient", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setVec3(shader, uniform_name, light->ambient);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.specular", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setVec3(shader, uniform_name, light->specular);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.diffuse", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setVec3(shader, uniform_name, light->diffuse);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.theta_min", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setFloat(shader, uniform_name, light->theta_min);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.theta_taper_start", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setFloat(shader, uniform_name, light->theta_taper_start);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.constant", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setFloat(shader, uniform_name, light->constant);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.linear", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setFloat(shader, uniform_name, light->linear);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.quadratic", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setFloat(shader, uniform_name, light->quadratic);

    retval = snprintf(uniform_name, max_unif_name,
                      "%s.shadow_matrix", light->name);
    if (retval >= max_unif_name || retval < 0){
        err_print("snprintf error");
        return LIGHT_ERR;
    }
    setMat4x4(shader, uniform_name, light->shadow_matrix);

    if (light->cube_mats){
        for (int i = 0; i < 6; i++){
            retval = snprintf(uniform_name, max_unif_name,
                              "%s.cube_mats[%i]", light->name, i);
            if (retval >= max_unif_name || retval < 0){
                err_print("snprintf error");
                return LIGHT_ERR;
            }
            setMat4x4(shader, uniform_name, light->cube_mats[i]);
        }
    }

    return LIGHT_SUCCESS;
}


light_error_t light_shadow_gl_init(Light * light)
{
    if (light->depth_texture){
        err_print("depth texture is not 0");
        return LIGHT_ERR;
    }
    glGenTextures(1, &light->depth_texture);
    glBindTexture(GL_TEXTURE_2D, light->depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, light->shadow_width,
                 light->shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const vec3 border_color = {1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (light->depth_FBO){
        err_print("depth FBO is not 0");
        return LIGHT_ERR;
    }
    glGenFramebuffers(1, &light->depth_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light->depth_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           light->depth_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return LIGHT_SUCCESS;
}


light_error_t light_shadow_cube_map_init(Light * light)
{
    light_error_t result = LIGHT_SUCCESS;
    if (light->depth_texture){
        err_print("depth texture is not 0");
        result = LIGHT_ERR;
    }
    glGenTextures(1, &light->depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, light->depth_texture);
    for (unsigned int i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     light->shadow_width, light->shadow_height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                        GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                        GL_CLAMP_TO_EDGE);
    }

    if (light->depth_FBO){
        err_print("depth FBO is not 0");
        result = LIGHT_ERR;
    }
    glGenFramebuffers(1, &light->depth_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light->depth_FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         light->depth_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (light->cube_mats){
        err_print("cube_mats is not NULL");
        result = LIGHT_ERR;
    }
    light->cube_mats = malloc(6 * sizeof(mat4x4));
    if (!light->cube_mats){
        err_print("Out of memory");
        result = LIGHT_ERR;
    }

    return result;
}


light_error_t light_shadow_mat_directional(Light * light, vec3 center, vec3 up,
                                           float near_plane, float far_plane,
                                           vec4 ortho_params)
{
    mat4x4 look_at;
    mat4x4 ortho;

    if (!light){
        err_print("light unallocated");
        return LIGHT_ERR;
    }
    mat4x4_look_at(look_at, light->position, center, up);
    mat4x4_ortho(ortho, ortho_params[0], ortho_params[1], ortho_params[2],
                 ortho_params[3], near_plane, far_plane);
    mat4x4_mul(light->shadow_matrix, ortho, look_at);
    return LIGHT_SUCCESS;
}


light_error_t light_shadow_cube_mat(Light * light, float near, float far)
{
    float aspect = (float)light->shadow_width / (float)light->shadow_height;
    mat4x4 shadow_proj;
    mat4x4_perspective(shadow_proj, (float)(90.f * M_PI / 180.f),
                       aspect, near, far);
    mat4x4 look_at;
    vec3 x_dir     = {1.f, 0.f, 0.f};
    vec3 y_dir     = {0.f, 1.f, 0.f};
    vec3 neg_y_dir = {0.f, -1.f, 0.f};
    vec3 z_dir     = {0.f, 0.f, 1.f};
    vec3 forward, up;

    vec3_add(forward, light->position, x_dir);
    mat4x4_look_at(look_at, light->position, forward, neg_y_dir);
    mat4x4_mul(light->cube_mats[0], shadow_proj, look_at);
    vec3_sub(forward, light->position, x_dir);
    mat4x4_look_at(look_at, light->position, forward, neg_y_dir);
    mat4x4_mul(light->cube_mats[1], shadow_proj, look_at);
    vec3_add(forward, light->position, y_dir);
    mat4x4_look_at(look_at, light->position, forward, z_dir);
    mat4x4_mul(light->cube_mats[2], shadow_proj, look_at);
    vec3_sub(forward, light->position, y_dir);
    mat4x4_look_at(look_at, light->position, forward, z_dir);
    mat4x4_mul(light->cube_mats[3], shadow_proj, look_at);
    vec3_add(forward, light->position, z_dir);
    mat4x4_look_at(look_at, light->position, forward, neg_y_dir);
    mat4x4_mul(light->cube_mats[4], shadow_proj, look_at);
    vec3_sub(forward, light->position, z_dir);
    mat4x4_look_at(look_at, light->position, forward, neg_y_dir);
    mat4x4_mul(light->cube_mats[5], shadow_proj, look_at);
}
