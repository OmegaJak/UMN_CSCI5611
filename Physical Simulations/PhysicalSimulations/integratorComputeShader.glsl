#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

precision highp float;

layout(std140, binding = 1) buffer Pos {
    vec4 Positions[];
};

layout(std140, binding = 2) buffer Vel {
    vec4 Velocities[];
};

layout(std140, binding = 5) buffer NewVel {
    vec4 NewVelocities[];
};

struct Connections {
    uint left, right, up, down;
};

struct MassParams {
    bool isFixed;
    float mass;
    Connections connections;
};

layout(std430, binding = 6) buffer MssPrps {
    MassParams MassParameters[];
};

layout(std430, binding = 4) buffer Parameters {
    float dt;
};

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uint gid;

void main() {
    gid = gl_GlobalInvocationID.x;

    Velocities[gid].xyz = NewVelocities[gid].xyz;

    if (!MassParameters[gid].isFixed) {
        Positions[gid].xyz += Velocities[gid].xyz * dt;
    } else {
        Velocities[gid].xyz = vec3(0, 0, 0);
    }
}