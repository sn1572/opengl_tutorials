#include <light.h>


light_error_t light_init(Light * light)
{
    if (light){
        light->name = NULL;
        light->position[0] = 0.f;
        light->position[1] = 0.f;
        light->position[2] = 0.f;
        light->direction[0] = 0.f;
        light->direction[1] = 0.f;
        light->direction[2] = 0.f;
        light->ambient[0] = 0.f;
        light->ambient[1] = 0.f;
        light->ambient[2] = 0.f;
        light->diffuse[0] = 0.f;
        light->diffuse[1] = 0.f;
        light->diffuse[2] = 0.f;
        light->specular[0] = 0.f;
        light->specular[1] = 0.f;
        light->specular[2] = 0.f;
        light->theta_min = 0.5f;
        light->theta_taper_start = 0.6f;
        light->constant = 1.0f;
        light->linear = 0.07f;
        light->quadratic = 0.017f;
        light->depth_FBO = 0;
        light->depth_texture = 0;
        light->shadow_width = 1024;
        light->shadow_height = 1024;
        return LIGHT_SUCCESS;
    } else{
        err_print("Attempting to initialize unallocated pointer");
        return LIGHT_ERR;
    }
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

    return LIGHT_SUCCESS;
}


light_error_t light_shadow_gl_init(Light * light)
{
    glGenTextures(1, &light->depth_texture);
    glBindTexture(GL_TEXTURE_2D, light->depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, light->shadow_width,
                 light->shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenFramebuffers(1, &light->depth_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light->depth_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           light->depth_texture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

