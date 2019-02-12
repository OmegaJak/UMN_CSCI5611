#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform int texID;
uniform float specFactor;

const float ambient = .25;
void main() {
    vec4 color;
    if (texID == -1)
        color = vec4(Color, 1);
    else if (texID == 0)
        color = texture(tex0, texcoord);
    else if (texID == 1)
        color = texture(tex1, texcoord);
    else {
        outColor = vec4(1, 0, 0, 1);
        return;  // This was an error, stop lighting!
    }
    if (color.a < 1) discard;
    vec3 normal = normalize(vertNormal);
    vec3 diffuseC = color.rgb * max(dot(-lightDir, normal), 0.0);
    vec3 ambC = color.rgb * ambient;
    vec3 viewDir = normalize(-pos);  // We know the eye is at (0,0)! (Do you know why?)
    vec3 reflectDir = reflect(viewDir, normal);
    float spec = max(dot(reflectDir, lightDir), 0.0);
    if (dot(-lightDir, normal) <= 0.0) spec = 0;  // No highlight if we are not facing the light
    vec3 specC = specFactor * vec3(1.0, 1.0, 1.0) * pow(spec, 4);
    vec3 oColor = ambC + diffuseC + specC;
    outColor = vec4(oColor, 1);
}