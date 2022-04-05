#include <bgfx/bgfx.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdexcept>
#include <iostream>

class WindowManager {

public:

    WindowManager(int width, int height, const char* title) :
    m_ptr(glfwCreateWindow(width, height, title, nullptr, nullptr))
    {
        if (!m_ptr)
            throw std::runtime_error("Window creation failed");
    }

    ~WindowManager() {
        if (m_ptr)
            glfwDestroyWindow(m_ptr);
    }

    WindowManager(const WindowManager&) = delete;

    WindowManager(WindowManager&& other) : m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }

    WindowManager& operator=(WindowManager&& other) {
        m_ptr = other.m_ptr;
        other.m_ptr = nullptr;
        return *this;
    }

    GLFWwindow* ptr() const {
        return m_ptr;
    }

private:
    GLFWwindow* m_ptr {nullptr};

};

void errorCallback(int code, const char* message) {
    std::cerr << "[GLFW ERROR] CODE=" << code << " MSG='" << message << "'\n";
}

int main() {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << "\n";
        return 1;
    }

    {
    WindowManager window {1024, 768, "Hello bgfx!"};

    while (!glfwWindowShouldClose(window.ptr())) {
        glfwPollEvents();
    }
    }

    glfwTerminate();

    return 0; 
}