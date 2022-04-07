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

# preprocesamiento de shaders
function(add_bgfx_shader target_name source output type varyingdef_filepath)

    set(OUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${output})
    set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${source})
    set(BGFX_BINARY_DIR ${CMAKE_BINARY_DIR}/aux1/ext/bgfx.cmake)
    set(BGFX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/aux1/ext/bgfx.cmake/bgfx)

    add_custom_command(
        OUTPUT ${OUT_FILE}
        COMMAND ${BGFX_BINARY_DIR}/shaderc
        ARGS -f ${SRC_FILE}
            -o ${OUT_FILE}
            --type ${type}
            --platform linux
            --verbose
            -i ${BGFX_SOURCE_DIR}/src
            --varyingdef ${CMAKE_CURRENT_SOURCE_DIR}/${varyingdef_filepath}
            -p spirv
    )

    add_custom_target(${target_name} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${output})
    add_dependencies(${target_name} shaderc)
    
endfunction()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_bgfx_shader(v_simple_shader src/shaders/v_simple.sc shaders/v_simple.bin vertex src/shaders/varying.def.sc)
add_bgfx_shader(f_simple_shader src/shaders/f_simple.sc shaders/f_simple.bin fragment src/shaders/varying.def.sc)


add_executable(hello_bgfx src/hello_bgfx.cpp)
target_link_libraries(hello_bgfx PUBLIC bx bgfx glfw) # bx se incluye porque sus headers son una dependencia de bgfx/platform.h
add_dependencies(hello_bgfx v_simple_shader f_simple_shader)

add_custom_target(Aux1)
add_dependencies(Aux1 hello_bgfx)