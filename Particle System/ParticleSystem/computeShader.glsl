#version 430 core
#extension GL_ARB_compute_shader: enable
#extension GL_ARB_shader_storage_buffer_object: enable

layout( std140, binding=1 ) buffer Pos {
    vec4 Positions[ ];
};

layout( std140, binding=2 ) buffer Vel {
    vec4 Velocities[ ];
};

layout( std140, binding=3 ) buffer Col {
    vec4 Colors[ ];
};

layout(std430, binding = 4) buffer Parameters {
    vec3 GravityCenter;
    float SimulationSpeed;
    float GravityFactor;
};

layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

const float timestep = 0.01;
const float G = 50;

void main() {
    uint gid = gl_GlobalInvocationID.x;
    float dt = timestep * SimulationSpeed;
    //const uvec3 idx = gl_WorkGroupID* gl_WorkGroupSize + gl_LocalInvocationID;

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;
    float r = length(GravityCenter - Positions[gid].xyz) / 5;
    vec3 a = (normalize(GravityCenter - Positions[gid].xyz) * (G + (1 / pow(r, 2)))) * GravityFactor;

    vec3 dta = dt * a;

    Positions[gid].xyz =p.xyz + v.xyz * dt + 0.5 * dt * dta;
    Velocities[gid].xyz = (v + dta) * 0.9999;
    
    /*float distFromCenter = length(Positions[gid].xyz - center);
    Colors[gid].xyz = vec3(distFromCenter, 0, distFromCenter);*/
}