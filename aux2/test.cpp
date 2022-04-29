#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "containers.hpp"

TEST_CASE("Invertir") {
    std::list<int> list_a {1, 5, 3, 8};
    std::list<int> list_b {8, 3, 5, 1};
    REQUIRE(invertir(list_a) == list_b);
}

TEST_CASE("RenderQueue") {
    
    RenderQueue queue;

    Object circle {1};
    Object triangle {2};
    Object square {3};

    Shader sh0 {1};
    Shader sh1 {2};

    queue.enqueue(circle, sh0);
    queue.enqueue(circle, sh1);
    queue.enqueue(triangle, sh0);
    queue.enqueue(triangle, sh0);
    queue.enqueue(square, sh1);
    queue.enqueue(square, sh0);
    queue.enqueue(triangle, sh0);

    queue.sort();

    REQUIRE(queue.drawCalls().size() == 7);

    Shader null_shader {0};
    Shader first_shader {queue.drawCalls()[0].shader};
    Shader second_shader {0};

    for (int i = 0; i < 7; ++i) {
        auto call = queue.drawCalls()[i];
        
        if (!second_shader) {
            if (call.shader != first_shader)
                second_shader = call.shader;
        } else
            REQUIRE(call.shader == second_shader);
    }

    queue.clear();
    REQUIRE(queue.drawCalls().size() == 0);
}