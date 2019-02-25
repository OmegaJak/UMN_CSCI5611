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
    float dt;
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

uint gid;
const float ks = 25;
const float kd = 10;
const float restLength = 5;
const vec3 gravity = vec3(0, 0, -9.8);

bool isNan(vec3 v) {
    return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

bool isInf(vec3 v) {
    return isinf(v.x) || isinf(v.y) || isinf(v.z);
}

void main() {
    gid = gl_GlobalInvocationID.x;
    if (Springs[gid].x < 0 || Springs[gid].y < 0) {
        return;
    }

    int massOne = Springs[gid].x;
    int massTwo = Springs[gid].y;
    
    // Things that would happen after a memory barrier, but I just place them at the start instead. Separate invocations act as the barrier
    //Velocities[massOne] = NewVelocities[massOne];
    //Velocities[massTwo] = NewVelocities[massTwo];

    //if (!MassParameters[massOne].isFixed) {
    //    Positions[massOne] += Velocities[massOne] * dt;
    //} else {
    //    Velocities[massOne].xyz = vec3(0, 0, 0);
    //}

    //if (!MassParameters[massTwo].isFixed) {
    //    Positions[massTwo] += Velocities[massTwo] * dt;
    //} else {
    //    Velocities[massTwo].xyz = vec3(0, 0, 0);
    //}
    //

    vec3 toMassOneFromTwo = Positions[massOne].xyz - Positions[massTwo].xyz;
    float length = length(toMassOneFromTwo);
    if (length == 0) {
        toMassOneFromTwo = vec3(0, 0, 1);
    } else {
        toMassOneFromTwo = toMassOneFromTwo / length;
    }

    float v1 = dot(toMassOneFromTwo, Velocities[massOne].xyz);
    float v2 = dot(toMassOneFromTwo, Velocities[massTwo].xyz);

    float springForce = -ks * (length - restLength);
    float dampForce = -kd * (v1 - v2);
    float force = springForce + dampForce;

    vec3 massOneAcc = gravity + 0.5 * force * toMassOneFromTwo / MassParameters[massOne].mass;
    vec3 massTwoAcc = gravity - 0.5 * force * toMassOneFromTwo / MassParameters[massTwo].mass;

    if (isInf(massOneAcc) || isNan(massOneAcc)) massOneAcc = vec3(0, 0, 0);
    if (isInf(massTwoAcc) || isNan(massTwoAcc)) massTwoAcc = vec3(0, 0, 0);

    barrier();

    NewVelocities[massOne].xyz += massOneAcc * dt;
    NewVelocities[massTwo].xyz += massTwoAcc * dt;
}