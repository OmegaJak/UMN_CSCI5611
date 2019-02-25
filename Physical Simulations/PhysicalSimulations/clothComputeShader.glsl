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
    int computationStage;
};

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uint gid;
const float ks = 30;
const float kd = 20;
const float restLength = 1;
const vec3 gravity = vec3(0, 0, -9.8);

bool isNan(vec3 v) {
    return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

bool isInf(vec3 v) {
    return isinf(v.x) || isinf(v.y) || isinf(v.z);
}

vec3 getAccelerationFromSpringConnection(uint massOne, uint massTwo) {
    if (massOne == 95683 || massTwo == 95683) return vec3(0, 0, 0);

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

    vec3 massOneAcc = 0.5 * force * toMassOneFromTwo / MassParameters[massOne].mass;

    if (isInf(massOneAcc) || isNan(massOneAcc)) massOneAcc = vec3(0, 0, 0);

    return massOneAcc;
}

void CalculateForces() {
    vec3 leftAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.left);
    vec3 rightAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.right);
    vec3 upAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.up);
    vec3 downAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.down);

    vec3 acc = gravity + leftAcc + rightAcc + upAcc + downAcc;

    NewVelocities[gid].xyz += acc * dt;
}

void ApplyForces() {
    Velocities[gid].xyz = NewVelocities[gid].xyz;

    if (!MassParameters[gid].isFixed) {
        Positions[gid].xyz += Velocities[gid].xyz * dt;
    } else {
        Velocities[gid].xyz = vec3(0, 0, 0);
    }
}

void main() {
    gid = gl_GlobalInvocationID.x;

    if (computationStage == 0) {
        CalculateForces();
    } else if (computationStage == 1) {
        ApplyForces();
    }
}