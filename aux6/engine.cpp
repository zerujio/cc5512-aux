#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "engine.hpp"
#include "cube.hpp"

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

MeshData createCubeMesh() {
    GLuint vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Cube::vertices), Cube::vertices, GL_STATIC_READ);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Cube::Vertex), nullptr);     // in vec3 a_position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Cube::Vertex),               // in vec3 a_normal
                          reinterpret_cast<void*>(offsetof(Cube::Vertex, normal)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Cube::indices), Cube::indices, GL_STATIC_READ);

    const GLsizei vertex_count = sizeof(Cube::vertices) / sizeof(Cube::Vertex);
    const GLsizei index_count = sizeof(Cube::indices) / sizeof(unsigned int);

    return {vao, vbo, ebo, vertex_count, index_count};
}

void updateTransforms(entt::registry& registry, SceneGraphNode& node, glm::mat4 parent_matrix = glm::mat4(1.0f)) {

    auto c_transform = registry.try_get<CTransform>(node.entity);

    glm::mat4 matrix = parent_matrix;

    if (c_transform) {
        matrix = glm::translate(matrix, c_transform->position);
        matrix = glm::rotate(matrix, c_transform->rotation.y, {0, 1, 0});
        matrix = glm::rotate(matrix, c_transform->rotation.x, {1, 0, 0});
        matrix = glm::rotate(matrix, c_transform->rotation.z, {0, 0, 1});
        matrix = glm::scale(matrix, c_transform->scale);

        c_transform->matrix = matrix;
    }

    for (auto& child : node.children) {
        updateTransforms(registry, child, matrix);
    }
}

glm::ivec2 window_size {800, 600};

void frameBufferSizeCallback(GLFWwindow* w, int width, int height) {
    window_size = {width, height};
    glViewport(0, 0, width, height);
}

void drawScene(Scene& scene) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glm::mat4 view_matrix = glm::lookAt(scene.camera.eye, scene.camera.at, scene.camera.up);
    glm::mat4 proj_matrix = glm::perspectiveFov(glm::pi<float>() / 4.0f, float(window_size.x), float(window_size.y), 0.001f, 1000.0f);

    // for each (CTransform, CVisual) in scene->registry
    scene.registry.view<CTransform, CVisual>().each(
    [&view_matrix, &proj_matrix](const CTransform& tr, const CVisual& vs) {
            //auto tr_matrix = glm::translate(glm::mat4 (1.0f), tr.position);

            constexpr int u_model_idx = 0;
            constexpr int u_view_idx = 1;
            constexpr int u_proj_idx = 2;
            constexpr int u_color_idx = 3;

            glUseProgram(vs.program->program);
            glUniformMatrix4fv(u_view_idx, 1, GL_FALSE, glm::value_ptr(view_matrix));
            glUniformMatrix4fv(u_proj_idx, 1, GL_FALSE, glm::value_ptr(proj_matrix));
            glUniformMatrix4fv(u_model_idx, 1, GL_FALSE, glm::value_ptr(tr.matrix));
            glUniform4fv(u_color_idx, 1, glm::value_ptr(vs.color));

            glBindVertexArray(vs.mesh->vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vs.mesh->ebo);

            glDrawElements(GL_TRIANGLES, vs.mesh->index_count, GL_UNSIGNED_INT, nullptr);
        }
    );
}

int main() {

    glfwSetErrorCallback(onGLFWError);

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    auto window = glfwCreateWindow(window_size.x, window_size.y, "Window", nullptr, nullptr);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    glDebugMessageCallback(onGLError, nullptr);

    glClearColor(0.05f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Scene scene;

    init(window, scene);

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double now = glfwGetTime();
        double delta = now - last;
        last = now;

        update(window, scene, delta);
        updateTransforms(scene.registry, scene.root);
        drawScene(scene);

        glfwSwapBuffers(window);
    }

    return 0;
}