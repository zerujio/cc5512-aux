#include "engine.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <forward_list>
#include <string>

struct SceneGraphNode {
    SceneGraphNode() = default;
    SceneGraphNode(entt::entity e) : entity(e) {}

    entt::entity entity {entt::null};
    std::forward_list<SceneGraphNode> children;
    std::string name;

    SceneGraphNode& findChild(std::string name);
};

// Escena
struct Scene {
    entt::registry registry;
    entt::entity player {registry.create()};
    SceneGraphNode root;
};

std::unique_ptr<Scene> scene;

// Recursos
struct RMesh {
    RMesh(GLuint vao_, GLuint ebo_, GLsizei index_count_)
    : vao(vao_), ebo(ebo_), index_count(index_count_) {}

    GLuint vao;
    GLuint ebo;
    GLsizei index_count;
};

struct RProgram {
    RProgram(GLuint p) : program(p) {}

    GLuint program;
};

// Componentes
struct CVisual {
    CVisual() = default;
    CVisual(glm::vec4 c, std::shared_ptr<RMesh> m, std::shared_ptr<RProgram> p)
    : color(c), mesh(std::move(m)), program(std::move(p)) {}

    glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
    std::shared_ptr<RMesh> mesh;
    std::shared_ptr<RProgram> program;
};

struct CTransform {
    glm::vec3 position;
    glm::mat4 matrix;
};

// Variables globales para rendering.
glm::mat4 view_matrix, proj_matrix;

constexpr int u_model_idx = 0;
constexpr int u_view_idx = 1;
constexpr int u_proj_idx = 2;
constexpr int u_color_idx = 3;

// Invocada cuando comienza la escena.
void init() {
    glClearColor(0.05f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    scene = std::make_unique<Scene>();

    auto program = std::make_shared<RProgram>(makeProgram("vert.glsl", "frag.glsl"));

    const auto mesh_data = createCubeMesh();
    auto mesh = std::make_shared<RMesh>(mesh_data.vao, mesh_data.ebo, mesh_data.index_count);

    glm::vec2 window_size = windowResolution();
    proj_matrix = glm::perspectiveFov(2.0f, window_size.x, window_size.y, 0.001f, 1000.0f);

    glm::vec3 camera_position {3.0f};
    glm::vec3 camera_target {0.0f};
    view_matrix = glm::lookAt(camera_position, camera_target, {0.0f, 1.0f, 0.0f});

    scene->root.entity = scene->player;
    scene->registry.emplace<CTransform>(scene->player);
    scene->registry.emplace<CVisual>(scene->player, glm::vec4(.25, .5, 1., 1.), mesh, program);

    auto obj1 = scene->registry.create();
    scene->root.children.emplace_front(obj1);
    scene->registry.emplace<CTransform>(obj1, glm::vec3(0., 0., 3.));
    scene->registry.emplace<CVisual>(obj1, glm::vec4(1., .5, 0., 1.), mesh, program);

    auto obj2 = scene->registry.create();
    scene->root.children.front().children.emplace_front(obj2);
    scene->registry.emplace<CTransform>(obj2, glm::vec3(3., 0., 0.));
    scene->registry.emplace<CVisual>(obj2, glm::vec4(1., 0., 0., 1.), mesh, program);

    // root
    // |
    // obj1
    // |
    // obj2
}

void updateTransforms(entt::registry& registry, SceneGraphNode& node, glm::mat4 parent_transform = glm::mat4(1.0f)) {

    auto c_transform = registry.try_get<CTransform>(node.entity);
    glm::mat4 new_transform = c_transform ? glm::translate(parent_transform, c_transform->position) : parent_transform;

    if (c_transform) {
        c_transform->matrix = new_transform;
    }

    for (auto& child : node.children) {
        updateTransforms(registry, child, new_transform);
    }
}

// En cada cuadro
void update(double delta) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    static float alpha = 0.0f;
    constexpr float ang_spd = 1.0f;
    alpha += ang_spd * delta;
    auto& tr = scene->registry.get<CTransform>(scene->player).position = {0., std::sin(alpha), 0.};

    // update transforms
    updateTransforms(scene->registry, scene->root);

    // for each (CTransform, CVisual) in scene->registry
    scene->registry.view<CTransform, CVisual>().each(
            [](const CTransform& tr, const CVisual& vs) {
                //auto tr_matrix = glm::translate(glm::mat4 (1.0f), tr.position);

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
