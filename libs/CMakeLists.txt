#GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
add_subdirectory(glfw)

#GLAD
add_subdirectory(glad)

#GLM
add_subdirectory(glm)

#EnTT
add_subdirectory(entt)

# OpenAL
add_subdirectory(openal-soft)

#dr_libs
add_library(dr_libs INTERFACE)
target_include_directories(dr_libs INTERFACE dr_libs)