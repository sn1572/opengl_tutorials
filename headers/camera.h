#ifndef CAMERA_H 
	#define CAMERA_H 

	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>
	#include <linmath.h>
	#include "shader.h"


	#define toRadians(x) (M_PI*x/180)

	// Error handling stuff
	typedef enum {
		CAM_NO_ERR,
		CAM_NULL_PTR,
	} cameraError_t;

	const static char CAM_UNK_ERR_MSG[] = "Unknown";
	const static char CAM_NO_ERR_MSG[] = "No";
	const static char CAM_NULL_PTR_MSG[] = "NULL pointer";

	const char * cameraGetErrorString(cameraError_t err){
		switch(err){
			case CAM_NO_ERR:
				return(CAM_NO_ERR_MSG);
				break;
			case CAM_NULL_PTR:
				return(CAM_NULL_PTR_MSG);
				break;
			default:
				return(CAM_UNK_ERR_MSG);
				break;
		}
	}

	static void cameraErrorInterpreter(cameraError_t err, const char * file, int line){
		if (err != CAM_NO_ERR){
			fprintf(stderr, "%s error at line %d of file %s\n", 
				 cameraGetErrorString(err), line, file);
			exit(1);
		}
	}

	#define camErrorHandler(err) (cameraErrorInterpreter(err, __FILE__, __LINE__))


	// Default camera values
	const static float YAW         		= -90.0f;
	const static float PITCH       		=  0.0f;
	const static float SPEED       		=  2.5f;
	const static float SENSITIVITY 		=  0.1f;
	const static float ZOOM        		=  45.0f;
	// Module local variables
	static struct Camera * _active_cam 	= NULL;


	struct Camera {
		// camera Attributes
		vec3 * position;
		vec3 * front;
		vec3 * up;
		vec3 * right;
		// mouse update storage
		float lastUpdateTime;
		float lastMouseX;
		float lastMouseY;
		// first mouse call flag
		bool firstMouse;
		// euler Angles
		float yaw;
		float pitch;
		// camera options
		float movementSpeed;
		float mouseSensitivity;
		float zoom;
		float aspect;
		float nearClipPlane;
		float farClipPlane;
		// Funcs
		void (*setViewMatrix)(struct Camera * self,
				       struct Shader * shaders,
				       const char * handle);
		void (*setProjectionMatrix)(struct Camera * self,
					     struct Shader * shaders,
					     const char * handle);
		void (*processKeyboard)(struct Camera * self,
					 GLFWwindow * window);
		void (*processMouseMovement)(struct Camera * self,
					      GLFWwindow * window,
					      double xoffset,
					      double yoffset);
		void (*processMouseScroll)(struct Camera * self,
					    GLFWwindow * window,
					    double yoffset);
		};


	// Module export functions
	void setActiveCamera(struct Camera * cam);
	void glfwCompatKeyboardCallback(GLFWwindow * window);
	void glfwCompatMouseMovementCallback(GLFWwindow * window, double xPos, double yPos);
	void glfwCompatMouseScrollCallback(GLFWwindow * window, double xPos, double yPos);
	void cameraFree(struct Camera * cam);
	cameraError_t setCameraPosition(struct Camera * cam, float x, float y, float z);
	void setActiveCameraPosition(float x, float y, float z);
	struct Camera * cameraInit(int width, int height);
	void sendMatrixToShader(mat4x4 matrix, const char * name, struct Shader * shaders);

	// Camera struct functions
	static void setViewMatrix(struct Camera * self, struct Shader * shaders, const char * handle);
	static void setProjectionMatrix(struct Camera * self, struct Shader * shaders, const char * handle);
	static void processKeyboard(struct Camera * self, GLFWwindow * window);
	static void processMouseMovement(struct Camera * self, GLFWwindow * window, double xoffset, double yoffset);
	static void processMouseScroll(struct Camera * self, GLFWwindow * window, double yoffset);

	// Module local functions
	static void updateCameraVectors(struct Camera * self);
	static cameraError_t flatten(float * out, mat4x4 M);
#endif
