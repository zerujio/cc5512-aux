# BGFX

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "")
set(BGFX_INSTALL OFF CACHE BOOL "")
set(BGFX_INSTALL_EXAMPLES OFF CACHE BOOL "")
set(BGFX_AMALGAMATED ON CACHE BOOL "")
set(BX_AMALGAMATED ON CACHE BOOL "")

add_subdirectory(ext/bgfx.cmake)

# GLFW

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(ext/glfw)

# Aux 1
add_executable(hello_bgfx src/hello_bgfx.cpp)
target_link_libraries(hello_bgfx PUBLIC bgfx glfw)

add_custom_target(Aux1)
add_dependencies(Aux1 hello_bgfx)