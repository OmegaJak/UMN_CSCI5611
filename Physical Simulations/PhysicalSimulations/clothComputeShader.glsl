#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std140, binding = 1) buffer Pos {
    vec4 Positions[];
};

layout(std140, binding = 2) buffer Vel {
    vec4 Velocities[];
};

layout(std140, binding = 5) buffer NewVel {
    vec4 NewVelocities[];
};

struct MassParams {
    bool isFixed;
    float mass;
};

layout(std430, binding = 6) buffer MssPrps {
    MassParams MassParameters[];
};

layout(std430, binding = 3) buffer Sprg {
    ivec2 Springs[];
};

layout(std430, binding = 4) buffer Parameters {
    int idk;
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

uint gid;
const float timestep = 0.00001;
const float ks = 100;
const float kd = 1;
const float restLength = 3;
const vec3 gravity = vec3(0, 0, -9.8);

void main() {
    gid = gl_GlobalInvocationID.x;
    if (Springs[gid].x < 0 || Springs[gid].y < 0) {
        return;
    }

    int massOne = Springs[gid].x;
    int massTwo = Springs[gid].y;
    float dt = timestep;

    vec3 toMassOneFromTwo = Positions[massOne].xyz - Positions[massTwo].xyz;
    float length = length(toMassOneFromTwo);
    toMassOneFromTwo = toMassOneFromTwo / length;

    float v1 = dot(toMassOneFromTwo, Velocities[massOne].xyz);
    float v2 = dot(toMassOneFromTwo, Velocities[massTwo].xyz);

    float springForce = -ks * (length - restLength);
    float dampForce = -kd * (v1 - v2);
    float force = springForce + dampForce;

    vec3 massOneAcc = gravity + 0.5 * force * toMassOneFromTwo / MassParameters[massOne].mass;
    vec3 massTwoAcc = gravity - 0.5 * force * toMassOneFromTwo / MassParameters[massTwo].mass;

    NewVelocities[massOne].xyz += massOneAcc * dt;
    NewVelocities[massTwo].xyz += massTwoAcc * dt;

    barrier();

    if (!MassParameters[massOne].isFixed) {
        Positions[massOne] += NewVelocities[massOne] * dt;
    } else {
        NewVelocities[massOne].xyz = vec3(0, 0, 0);
    }

    if (!MassParameters[massTwo].isFixed) {
        Positions[massTwo] += NewVelocities[massTwo] * dt;
    } else {
        NewVelocities[massTwo].xyz = vec3(0, 0, 0);
    }

    Velocities[massOne] = NewVelocities[massOne];
    Velocities[massTwo] = NewVelocities[massTwo];
}