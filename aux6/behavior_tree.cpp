#include "engine.hpp"

#include <vector>

class BTNode {
public:
    enum class Status {
        Failure, Success, Running
    };

    virtual Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) = 0;
};

class BTSequence : public BTNode {
public:
    BTSequence() = default;

    BTSequence(std::vector<std::unique_ptr<BTNode>> sn) : subnodes(std::move(sn)) {}

    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        for (auto& bt : subnodes) {
            auto status = bt->tick(scene, entity, delta, window);
            if (status != Status::Success)
                return status;
        }
        return Status::Success;
    }

    std::vector<std::unique_ptr<BTNode>> subnodes;
};

class BTFallback : public BTNode {
public:
    BTFallback() = default;

    BTFallback(std::vector<std::unique_ptr<BTNode>> sn) : subnodes(std::move(sn)) {}

    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        Status status;
        for (auto& bt : subnodes) {
            status = bt->tick(scene, entity, delta, window);
            if (status != Status::Failure)
                return status;
        }
        return status;
    }

    std::vector<std::unique_ptr<BTNode>> subnodes;
};

class BTMove : public BTNode {
public:
    BTMove() = default;

    BTMove(glm::vec3 v) : velocity(v) {}

    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        auto& tr = scene.registry.get<CTransform>(entity);
        tr.position += velocity * delta;

        return Status::Success;
    }

    glm::vec3 velocity;
};

class BTCheckInput : public BTNode {
public:
    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        glm::ivec2 input (
                glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A),
                glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S)
        );

        return input.x or input.y ? Status::Success : Status::Failure;
    }
};

class BTMoveFromInput : public BTNode {
public:
    BTMoveFromInput(float spd) : speed(spd) {}

    Status tick(Scene& scene, entt::entity entity, float delta, GLFWwindow* window) override {
        glm::vec3 input (
                glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A),
                glfwGetKey(window, GLFW_KEY_W) - glfwGetKey(window, GLFW_KEY_S),
                0.0f
        );
        bt_move.velocity = glm::normalize(input) * speed;
        return bt_move.tick(scene, entity, delta, window);
    }

    float speed {1.0f};

private:
    BTMove bt_move;
};

class BTIsNear : public BTNode {
public:
    BTIsNear(entt::entity tg, float dist) : target(tg), distance(dist) {}
    BTIsNear(entt::entity tg) : target(tg) {}

    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        auto& this_tr = scene.registry.get<CTransform>(entity);
        auto& tg_tr = scene.registry.get<CTransform>(target);

        return glm::length(this_tr.position - tg_tr.position) < distance ? Status::Success : Status::Failure;
    }

    entt::entity target;
    float distance {1.0f};
};

class BTMoveTowards : public BTNode {
public:
    BTMoveTowards(entt::entity tg, float spd) : target(tg), speed(spd) {}
    BTMoveTowards(entt::entity tg) : target(tg) {}

    Status tick(Scene &scene, entt::entity entity, float delta, GLFWwindow *window) override {
        auto& this_tr = scene.registry.get<CTransform>(entity);
        auto& tg_tr = scene.registry.get<CTransform>(target);

        bt_move.velocity = glm::normalize(tg_tr.position - this_tr.position) * speed;
        return bt_move.tick(scene, entity, delta, window);
    }

    entt::entity target;
    float speed {1.0f};
private:
    BTMove bt_move;
};


struct CBehaviorTree {
    std::unique_ptr<BTNode> root;
};

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

    auto red = spawnCube({1.0f, 0.0f, 0.0f});
    scene.registry.get<CTransform>(red).position = {-1.0f, 0.0f, 0.0f};
    {
        /* Seq
         * |      \
         * Input?  MoveFromInput
         *              |
         *             Move
         */

        float speed = 2.0f;

        auto check_input = std::make_unique<BTCheckInput>();
        auto move_from_input = std::make_unique<BTMoveFromInput>(speed);
        auto seq = std::make_unique<BTSequence>();
        seq->subnodes.emplace_back(std::move(check_input));
        seq->subnodes.emplace_back(std::move(move_from_input));
        scene.registry.emplace<CBehaviorTree>(red).root = std::move(seq);
    }

    auto green = spawnCube({.0f, 1.0f, .0f});
    {
        /*
         * Fallback
         *  |       \
         * IsNear   MoveTowards
         */

        float speed = 1.5f;
        float distance = 1.5f;

        auto is_near = std::make_unique<BTIsNear>(red, distance);
        auto move_towards = std::make_unique<BTMoveTowards>(red, speed);
        auto fallback = std::make_unique<BTFallback>();
        fallback->subnodes.emplace_back(std::move(is_near));
        fallback->subnodes.emplace_back(std::move(move_towards));
        scene.registry.emplace<CBehaviorTree>(green).root = std::move(fallback);
    }

    auto blue = spawnCube({.0f, .0f, 1.0});
    scene.registry.get<CTransform>(blue).position = {1.0f, 0.0f, 0.0f};
    {
        /*
         * Sequence
         *  |       \
         * IsNear   MoveTowards
         */

        float speed = 2.5f;
        float distance = 1.25f;

        auto is_near = std::make_unique<BTIsNear>(red, distance);
        auto move_towards = std::make_unique<BTMoveTowards>(red, -speed);
        auto sequence = std::make_unique<BTSequence>();
        sequence->subnodes.emplace_back(std::move(is_near));
        sequence->subnodes.emplace_back(std::move(move_towards));
        scene.registry.emplace<CBehaviorTree>(blue).root = std::move(sequence);
    }

    scene.camera.eye = {.0f, .0f, 10.0f};
    scene.camera.at = {.0f, .0f, .0f};
}

void update(GLFWwindow* window, Scene& scene, double delta) {

    // update behavior trees
    scene.registry.view<CBehaviorTree>().each(
            [window, &scene, delta](entt::entity entity, const CBehaviorTree &bt_component) {
                bt_component.root->tick(scene, entity, float(delta), window);
            }
    );
}
