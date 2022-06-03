#ifndef AUX4__ENGINE_HPP
#define AUX4__ENGINE_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt/entt.hpp>

#include <string>
#include <memory>
#include <forward_list>
#include <string>

// utilidades
GLuint loadShader(const std::string &path, GLenum shader_type);
void linkProgram(GLuint program, GLuint vertex, GLuint fragment);
GLuint makeProgram(const std::string & vertex, const std::string & fragment);

struct MeshData {
    GLuint vao, vbo, ebo;
    GLsizei vertex_count, index_count;
};
MeshData createCubeMesh();

struct SceneGraphNode {
    SceneGraphNode() = default;
    SceneGraphNode(entt::entity e) : entity(e) {}

    entt::entity entity {entt::null};
    std::forward_list<SceneGraphNode> children;
    std::string name;
};

struct Camera {
    glm::vec3 eye {1, 1, 1};
    glm::vec3 at {0, 0, 0};
    glm::vec3 up {0, 1, 0};
};

// Escena
struct Scene {
    entt::registry registry;
    entt::entity player {registry.create()};
    SceneGraphNode root;
    Camera camera;
};

// Recursos

// Mesh
struct RMesh {
    RMesh(GLuint vao_, GLuint ebo_, GLsizei index_count_)
            : vao(vao_), ebo(ebo_), index_count(index_count_) {}

    GLuint vao;
    GLuint ebo;
    GLsizei index_count;
};

// Shader program
struct RProgram {
    RProgram(GLuint p) : program(p) {}

    GLuint program;
};

// Componentes

// Visual
struct CVisual {
    CVisual() = default;
    CVisual(glm::vec4 c, std::shared_ptr<RMesh> m, std::shared_ptr<RProgram> p)
            : color(c), mesh(std::move(m)), program(std::move(p)) {}

    glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
    std::shared_ptr<RMesh> mesh;
    std::shared_ptr<RProgram> program;
};

// Transformación
struct CTransform {
    glm::vec3 position {0, 0, 0};
    glm::vec3 rotation {0, 0, 0};
    glm::vec3 scale {1, 1, 1};
    glm::mat4 matrix {1.0f};
};

// definidas por el usuario
void init(GLFWwindow* window, Scene& scene);
void update(Scene &scene, double delta);

#endif //AUX4__ENGINE_HPP
