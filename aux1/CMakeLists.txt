add_executable(hello_bgfx src/hello_bgfx)

add_custom_target(Aux1)
add_dependencies(Aux1 hello_bgfx)