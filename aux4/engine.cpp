#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "engine.hpp"

void onGLFWError(int error_code, const char* description) {
    std::cout << "[GLFW ERROR] " << error_code << " : " << description << std::endl;
}

void onGLError(GLenum source,
               GLenum type,
               GLuint id,
               GLenum severity,
               GLsizei length,
               const GLchar *message,
               const void *userParam) {
    const char *source_string;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            source_string = "OpenGL API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            source_string = "Window-system API";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            source_string = "Shader compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            source_string = "Third-party application";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            source_string = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            source_string = "Other";
            break;
        default:
            source_string = "Unknown/Invalid";
            break;
    }

    const char *type_string;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            type_string = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            type_string = "Deprecated behavior";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            type_string = "Undefined behavior";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            type_string = "Portability warning";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type_string = "Performance warning";
            break;
        case GL_DEBUG_TYPE_MARKER:
            type_string = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            type_string = "Push group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            type_string = "Pop group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            type_string = "Other";
            break;
        default:
            type_string = "Unknown/Invalid";
            break;
    }

    const char *severity_string;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severity_string = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_string = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severity_string = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severity_string = "Notification";
            break;
        default:
            severity_string = "Unknown/Invalid";
            break;
    }

    std::cout << "\n[OpenGL DEBUG MESSAGE]\n"
              << "Source  :" << source_string << '\n'
              << "Type    :" << type_string << '\n'
              << "Severity:" << severity_string << '\n'
              << message << std::endl;
}

GLuint loadShader(const std::string &path, GLenum shader_type) {
    std::fstream file {path};
    auto shader = glCreateShader(shader_type);
    auto src = (std::stringstream() << file.rdbuf()).str();
    auto c_str = src.c_str();
    glShaderSource(shader, 1, &c_str, nullptr);
    glCompileShader(shader);
    return shader;
}

void linkProgram(GLuint program, GLuint vertex, GLuint fragment) {
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
}

GLuint makeProgram(const std::string & vertex, const std::string & fragment) {
    auto v = loadShader(vertex, GL_VERTEX_SHADER);
    auto f = loadShader(fragment, GL_FRAGMENT_SHADER);
    auto program = glCreateProgram();
    linkProgram(program, v, f);
    glDeleteShader(f);
    glDeleteShader(v);
    return program;
}

glm::ivec2 windowResolution() {
    return {800, 600};
}

int main() {

    glfwSetErrorCallback(onGLFWError);

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    auto wsize = windowResolution();
    auto window = glfwCreateWindow(wsize.x, wsize.y, "Window", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glDebugMessageCallback(onGLError, nullptr);

    init();

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        double now = glfwGetTime();
        double delta = now - last;
        last = now;
        update(delta);
        glfwSwapBuffers(window);
    }

    return 0;
}