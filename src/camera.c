#include <camera.h>


static const size_t vecSize = 3*sizeof(float);


// `exported` functions.
void setActiveCamera(struct Camera * cam){
    _active_cam = cam;
}


void glfwCompatKeyboardCallback(GLFWwindow * window){
    _active_cam->processKeyboard(_active_cam, window);
}


void glfwCompatMouseMovementCallback(GLFWwindow * window, double xPos,
                                     double yPos){
    _active_cam->processMouseMovement(_active_cam, window, xPos, yPos);
}


void glfwCompatMouseScrollCallback(GLFWwindow * window, double xPos,
                                   double yPos){
    _active_cam->processMouseScroll(_active_cam, window, yPos);
}


cameraError_t flatten(float * out, mat4x4 M){
    if (!out)
        return CAM_NULL_PTR;
    if (!M)
        return CAM_NULL_PTR;
    for (int i=0; i<4; i++){
        for (int j=0; j<4; j++){
            out[i*4+j] = M[i][j];
        }
    }
    return CAM_NO_ERR;
}


void sendMatrixToShader(mat4x4 matrix, const char * name,
                        struct Shader * shaders){
    GLint loc = glGetUniformLocation(shaders->ID, name);
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "Error in %s, line %d in file %s\n",
                __func__, __LINE__-4, __FILE__);
        fprintf(stderr, "GL error %x\n", glError); 
        exit(1);
    }
    float flattened[16];
    flatten(flattened, matrix);
    glUniformMatrix4fv(loc, 1, GL_FALSE, flattened);
    if (glError != GL_NO_ERROR){
        fprintf(stderr, "Error in %s, line %d in file %s\n",
                __func__, __LINE__-3, __FILE__);
        fprintf(stderr, "GL error %x\n", glError); 
        exit(1);
    }
}


void setViewMatrix(struct Camera * self, struct Shader * shaders,
                          const char * handle){
    vec3 temp;
    mat4x4 view;
    if (!(self->position) || !(self->front)){
        printf("NULL ptr in %s", __func__);
        // Really need a cleanup / bail-out function here...
    }
    vec3_add(temp, *(self->position), *(self->front));
    mat4x4_look_at(view, *(self->position), temp, *(self->up));
    sendMatrixToShader(view, handle, shaders);
}


void setProjectionMatrix(struct Camera * self, struct Shader * shaders,
                                const char * handle){
    mat4x4 projection;
    mat4x4_perspective(projection,
                       self->zoom*M_PI/180,
                       self->aspect,
                       self->nearClipPlane,
                       self->farClipPlane);
    sendMatrixToShader(projection, handle, shaders);
}


void processKeyboard(struct Camera * self, GLFWwindow * window){
                            float deltaTime, currentTime;

    currentTime = glfwGetTime();
    deltaTime = currentTime-(self->lastUpdateTime);
    self->lastUpdateTime = currentTime;

    float velocity = (self->movementSpeed) * deltaTime;
    vec3 position, front, right;
    memcpy(position, *(self->position), vecSize);
    memcpy(front, *(self->front), vecSize);
    memcpy(right, *(self->right), vecSize);
    vec3_scale(front, front, velocity);
    vec3_scale(right, right, velocity);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        vec3_add(position, position, front);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        vec3_sub(position, position, right);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        vec3_sub(position, position, front);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        vec3_add(position, position, right);
    }

    //self->position = &position;
    memcpy(*(self->position), position, vecSize);
}


void processMouseMovement(struct Camera * self, GLFWwindow * window,
                                 double xPos, double yPos){
    if (self->firstMouse){
        self->lastMouseX = xPos;
        self->lastMouseY = yPos;
        self->firstMouse = false;
    }

    float xOffset = xPos-self->lastMouseX;
    float yOffset = self->lastMouseY-yPos;
    self->lastMouseX = xPos;
    self->lastMouseY = yPos;

    xOffset *= self->mouseSensitivity;
    yOffset *= self->mouseSensitivity;

    self->yaw += xOffset;
    self->pitch += yOffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (self->pitch > 89.0)
        self->pitch = 89.0;
    if (self->pitch < -89.0)
        self->pitch = -89.0;

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors(self);
}


void processMouseScroll(struct Camera * self, GLFWwindow * window,
                               double yoffset){
    float zoom = self->zoom;
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 60.0f)
        zoom = 60.0f; 
    self->zoom = zoom;
}


static void updateCameraVectors(struct Camera * self){
// calculate the new Front vector
    float yaw = self->yaw;
    float pitch = self->pitch;  
    float x, y, z;
    x = cos(toRadians(yaw)) * cos(toRadians(pitch));
    y = sin(toRadians(pitch));
    z = sin(toRadians(yaw)) * cos(toRadians(pitch));
    vec3 front = {x,y,z};
    vec3_norm(front, front);
    memcpy(*(self->front), front, vecSize);

    vec3 right;
    vec3_mul_cross(right, *(self->front), *(self->up));
    vec3_norm(right, right);
    memcpy(*(self->right), right, vecSize);

    /*
    vec3 up;
    vec3_mul_cross(up, right, front);
    vec3_norm(up, up);
    memcpy(*(self->up), up, vecSize);
    */
}


struct Camera * cameraInit(int width, int height){
    struct Camera * out = NULL;
    out = (struct Camera *)calloc(1, sizeof(struct Camera));

    out->position = NULL;
    out->position = (vec3 *)calloc(1, sizeof(vec3));
    out->front = NULL;
    out->front = (vec3 *)calloc(1, sizeof(vec3));
    out->up = NULL;
    out->up = (vec3 *)calloc(1, sizeof(vec3));
    out->right = NULL;
    out->right = (vec3 *)calloc(1, sizeof(vec3));

    vec3 * p = NULL, * f = NULL, * u = NULL, * r = NULL;
    p = out->position;
    f = out->front;
    u = out->up;
    r = out->right;

    if (!p || !f || !r || !u || !out){
        camErrorHandler(CAM_NULL_PTR);
    }

    vec3 initPos = {0,0,0};
    vec3 initFront = {0,0,-1};
    vec3 initUp = {0,1,0};
    vec3 initRight = {1,0,0};

    memcpy(*(out->position), initPos, vecSize);
    memcpy(*(out->front), initFront, vecSize);
    memcpy(*(out->up), initUp, vecSize);
    memcpy(*(out->right), initRight, vecSize);

    // Currently no `good` way to check if glfw
    // has been initialized.
    out->lastUpdateTime = glfwGetTime();
    out->lastMouseX = width/2;
    out->lastMouseY = height/2;
    out->aspect = width/height;
    out->nearClipPlane = 0.1;
    out->farClipPlane = 100.0;
    out->firstMouse = true;

    out->yaw = YAW;
    out->pitch = PITCH;
    out->movementSpeed = SPEED;
    out->mouseSensitivity = SENSITIVITY;
    out->zoom = ZOOM;

    out->setViewMatrix = setViewMatrix;
    out->setProjectionMatrix = setProjectionMatrix;
    out->processKeyboard = processKeyboard;
    out->processMouseMovement = processMouseMovement;
    out->processMouseScroll = processMouseScroll;

    return(out);
}


void cameraFree(struct Camera * cam){
    free(cam->position);
    free(cam->front);
    free(cam->up);
    free(cam->right);
    free(cam);
}


cameraError_t setCameraPosition(struct Camera * cam, float x, float y,
                                float z){
    vec3 setPos = {x,y,z};
    if (cam->position && *(cam->position)){
        // Ensuring we don't try to memcpy to a null ptr.
        // Results in anything from a segfault to a stack
        // smash to some insidious memory thrashing.
        memcpy(*(cam->position), setPos, vecSize);
        return(CAM_NO_ERR);
    }   
    else{
        return(CAM_NULL_PTR);
    }
}


void setActiveCameraPosition(float x, float y, float z){
    camErrorHandler(setCameraPosition(_active_cam, x, y, z));
}
