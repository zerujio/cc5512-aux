#include <bgfx/bgfx.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>


void errorCallback(int code, const char* message) {
    std::cerr << "[GLFW ERROR] CODE=" << code << " MSG='" << message << "'\n";
}

int main() {

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << "\n";
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Hello bgfx!", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed!" << "\n";
        return 2;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0; 
}