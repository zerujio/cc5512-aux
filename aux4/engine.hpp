#ifndef AUX4__ENGINE_HPP
#define AUX4__ENGINE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <memory>

// utilidades
GLuint loadShader(const std::string &path, GLenum shader_type);
void linkProgram(GLuint program, GLuint vertex, GLuint fragment);
GLuint makeProgram(const std::string & vertex, const std::string & fragment);

glm::ivec2 windowResolution();

struct MeshData {
    GLuint vao, vbo, ebo;
    GLsizei vertex_count, index_count;
};
MeshData createCubeMesh();

// definidas por el usuario
void init();
void update(double delta);

#endif //AUX4__ENGINE_HPP
