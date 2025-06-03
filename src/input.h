#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

void registerCallbacks(GLFWwindow* window);
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButton(GLFWwindow* window, int button, int action, int mods);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

#endif
