#version 460

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 position;
//out vec3 normal;

layout (location = 0) uniform mat4 u_model;
layout (location = 1) uniform mat4 u_view;
layout (location = 2) uniform mat4 u_proj;

void main() {
    gl_Position = u_proj * u_view * u_model * vec4(a_position, 1.0f);
    position = a_position;
    //normal = a_normal;
}
