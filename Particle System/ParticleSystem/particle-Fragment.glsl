#version 150 core

in vec4 Color;
out vec4 outColor;

uniform sampler2D tex0;

void main() {
    //outColor = texture(tex0, gl_PointCoord);
    outColor = Color;
}