#include "engine.hpp"
#include "cube.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt/entt.hpp>

#include <memory>

glm::mat4 view_matrix, proj_matrix;

GLuint program, vao, vbo, ebo;
GLsizei index_count;

constexpr int u_model_idx = 0;
constexpr int u_view_idx = 1;
constexpr int u_proj_idx = 2;
constexpr int u_color_idx = 3;

struct Scene {
    entt::registry registry;
    entt::entity player {entt::null};
};

std::unique_ptr<Scene> scene;

struct Visual {
    glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
};

struct Transform {
    glm::vec3 position;
};

void init() {
    glClearColor(0.05f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    program = makeProgram("vert.glsl", "frag.glsl");

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
    index_count = sizeof(Cube::indices) / sizeof(unsigned int);

    glm::vec2 window_size = windowResolution();
    proj_matrix = glm::perspectiveFov(2.0f, window_size.x, window_size.y, 0.001f, 1000.0f);

    glm::vec3 camera_position {3.0f};
    glm::vec3 camera_target {0.0f};
    view_matrix = glm::lookAt(camera_position, camera_target, {0.0f, 1.0f, 0.0f});

    glUseProgram(program);
    glUniformMatrix4fv(u_view_idx, 1, GL_FALSE, glm::value_ptr(view_matrix));
    glUniformMatrix4fv(u_proj_idx, 1, GL_FALSE, glm::value_ptr(proj_matrix));

    scene = std::make_unique<Scene>();

    auto obj1 = scene->registry.create();
    scene->registry.emplace<Transform>(obj1).position = {0., 0., 3.};
    scene->registry.emplace<Visual>(obj1).color = {1., .5, 0., 1.};

    auto obj2 = scene->registry.create();
    scene->registry.emplace<Transform>(obj2).position = {3., 0., 0.};
    scene->registry.emplace<Visual>(obj2).color = {1., 0., 0., 1.};

    scene->player = scene->registry.create();
    scene->registry.emplace<Transform>(scene->player);
    scene->registry.emplace<Visual>(scene->player).color = {.25, .5, 1., 1.};

}

void update(double delta) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    static float alpha = 0.0f;
    constexpr float ang_spd = 1.0f;
    alpha += ang_spd * delta;
    auto& tr = scene->registry.get<Transform>(scene->player).position = {0., std::sin(alpha), 0.};

    scene->registry.view<Transform, Visual>().each(
            [](const Transform& tr, const Visual& vs) {
                auto tr_matrix = glm::translate(glm::mat4 (1.0f), tr.position);
                glUniformMatrix4fv(u_model_idx, 1, GL_FALSE, glm::value_ptr(tr_matrix));
                glUniform4fv(u_color_idx, 1, glm::value_ptr(vs.color));
                glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
            }
    );
}
