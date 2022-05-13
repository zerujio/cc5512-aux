add_executable(test test.cpp containers.cpp)

add_custom_target(aux2)
add_dependencies(aux2 test)