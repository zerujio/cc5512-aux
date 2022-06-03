#version 460

//in vec3 normal;
out vec4 fragColor;

layout(location = 3) uniform vec4 u_color = {1.0f, 1.0f, 1.0f, 1.0f};

void main() {
    //fragColor = u_color * vec4(abs(normal), 1.0f);
    fragColor = u_color;
}
