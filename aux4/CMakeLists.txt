add_executable(scene_graph scene_graph.cpp engine.cpp)
target_link_libraries(scene_graph glfw glad glm EnTT::EnTT)

add_custom_target(aux4)
add_dependencies(aux4 scene_graph)

file(COPY frag.glsl vert.glsl DESTINATION .)