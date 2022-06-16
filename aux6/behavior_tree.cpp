#include "engine.hpp"

entt::entity red, green, blue;

float red_speed = 2.0f;
float green_speed = 1.5f;
float blue_speed = 2.5f;

float green_distance = 1.5f;
float blue_distance = 1.25f;

void init(GLFWwindow* window, Scene& scene) {
    auto shader_program = std::make_shared<RProgram>(makeProgram("vert.glsl", "frag.glsl"));
    auto mesh = std::make_shared<RMesh>(createCubeMesh());

    auto spawnCube = [&scene, &shader_program, &mesh] (const glm::vec3 &color) {
        auto e = scene.registry.create();
        scene.root.children.emplace_front(e);
        scene.registry.emplace<CVisual>(e, glm::vec4(color, 1.0f), mesh, shader_program);
        scene.registry.emplace<CTransform>(e);
        return e;
    };

    red = spawnCube({1.0f, 0.0f, 0.0f});
    scene.registry.get<CTransform>(red).position = {-1.0f, 0.0f, 0.0f};

    green = spawnCube({.0f, 1.0f, .0f});

    blue = spawnCube({.0f, .0f, 1.0f});
    scene.registry.get<CTransform>(blue).position = {1.0f, 0.0f, 0.0f};

    scene.camera.eye = {.0f, .0f, 10.0f};
    scene.camera.at = {.0f, .0f, .0f};
}

void update(GLFWwindow* window, Scene& scene, double delta) {

    // move red
    glm::vec2 input (
            glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A),
            glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S)
    );
    auto& red_tr = scene.registry.get<CTransform>(red);

    if (input.x or input.y)
        red_tr.position += glm::vec3(glm::normalize(input), 0.0f) * float(delta) * red_speed;

    // move green
    auto& green_tr = scene.registry.get<CTransform>(green);
    auto rg_dist = red_tr.position - green_tr.position;
    if (glm::length(rg_dist) > green_distance)
        green_tr.position += glm::normalize(rg_dist) * green_speed * float(delta);

    // move blue
    auto &blue_tr = scene.registry.get<CTransform>(blue);
    auto rb_dist = red_tr.position - blue_tr.position;
    if (glm::length(rb_dist) < blue_distance)
        blue_tr.position -= glm::normalize(rb_dist) * blue_speed * float(delta);
}
