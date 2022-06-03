#add_executable(hello_openal ejemplo.cpp)
#target_link_libraries(hello_openal OpenAL dr_libs EnTT::EnTT glm)

add_executable(spatial_audio spatial_audio.cpp engine.cpp)
target_link_libraries(spatial_audio glfw glad glm EnTT::EnTT OpenAL dr_libs)

add_custom_target(aux5)
add_dependencies(aux5 spatial_audio)

file(COPY frag.glsl vert.glsl loop.wav
    DESTINATION .)