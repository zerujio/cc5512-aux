add_executable(behavior_tree behavior_tree.cpp engine.cpp)
target_link_libraries(behavior_tree glfw glad glm EnTT::EnTT)

add_custom_target(aux6)
add_dependencies(aux6 behavior_tree)

file(COPY frag.glsl vert.glsl DESTINATION .)