#pragma once

//	#include <errno.h>
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL 
#endif

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define QUI __LINE__,__FILE__

// funzioni con controllo degli errori

void xterminate(const char* message, int line, const char* file);
void xassert(bool expression, const char* message, int line, const char* file);

// funzioni GLFW
#ifdef _glfw3_h_

int xglfwInit(int line, const char* file);

GLFWwindow* xglfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share, int linea, const char* file);

int xglfwGetKey(GLFWwindow* window, int key, int line, const char* file);

#endif