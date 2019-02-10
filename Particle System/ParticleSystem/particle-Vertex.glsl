#version 150 core

in vec3 position;
in vec3 inColor;

uniform mat4 view;
uniform mat4 proj;

out vec3 Color;

void main() {
    Color = inColor;
    gl_Position = proj * view * vec4(position, 1.0);
}