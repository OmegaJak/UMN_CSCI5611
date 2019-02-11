#version 150 core

in vec3 position;
in vec4 inColor;

uniform mat4 view;
uniform mat4 proj;
uniform vec2 screenSize;
uniform float spriteSize;

out vec4 Color;

void main() {
    Color = inColor;

    // Scale the billboard so it maintains correct world size
    // https://stackoverflow.com/questions/17397724/point-sprites-for-particle-system
    vec4 eyePos = view * vec4(position, 1.0);
    vec4 projVoxel = proj * vec4(spriteSize, spriteSize, eyePos.z, eyePos.w);
    vec2 projSize = (screenSize * projVoxel.xy) / projVoxel.w;
    gl_PointSize = 0.25 * (projSize.x + projSize.y);
    //gl_PointSize = 4;
    
    gl_Position = proj * eyePos;

}