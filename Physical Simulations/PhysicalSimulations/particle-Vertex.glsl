#version 150 core

in vec3 position;
in vec4 inColor;

uniform mat4 view;
uniform mat4 proj;

out vec4 Color;

void main() {
    Color = inColor;
    vec4 eyePos = view * vec4(position, 1.0);
    gl_Position = proj * eyePos;
}