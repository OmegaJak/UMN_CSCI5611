#version 150 core

in vec4 Color;
out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
    //outColor = texture(tex0, gl_PointCoord);
    vec4 color = Color;
    color = texture(tex2, gl_PointCoord);
    if (color.a < 0.5) discard;

    outColor = vec4(Color.rgb, 1);
}