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

layout(std140, binding = 3) buffer Norms {
    vec4 Normals[];
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
    float ks;
    float kd;
    float restLength;
};

uniform int computationStage;

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

uint gid;
const vec3 gravity = vec3(0, 0, -9.8);

// I'm rather sad that I need this.
// Because I need to be able to use both gid and the connection index as inputs, getAccelerationFromSpringConnection takes uints
// glsl can't convert uint to int
// I would prefer to use -1 as a bad index, as you can't index into an array with -1. But because I have to use uint's I've just gotta pick a large number
// I tried making a Connection data structure, but things broke when I did so. Maybe GLSL doesn't like structs in structs in structs. Data packing gets weird sometimes.
const uint BAD_INDEX = 8000001;

bool isNan(vec3 v) {
    return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

bool isInf(vec3 v) {
    return isinf(v.x) || isinf(v.y) || isinf(v.z);
}

vec3 getSpringAcceleration(vec3 p1, vec3 v1, float m1, vec3 p2, vec3 v2) {
    vec3 toMassOneFromTwo = p1 - p2;
    float length = length(toMassOneFromTwo);
    if (length == 0) {
        toMassOneFromTwo = vec3(0, 0, 1);
    } else {
        toMassOneFromTwo = toMassOneFromTwo / length;
    }

    float dampV1 = dot(toMassOneFromTwo, v1);
    float dampV2 = dot(toMassOneFromTwo, v2);

    float springForce = -ks * (length - restLength);
    float dampForce = -kd * (dampV1 - dampV2);
    float force = springForce + dampForce;

    vec3 massOneAcc = 0.5 * force * toMassOneFromTwo / m1;

    if (isInf(massOneAcc) || isNan(massOneAcc)) massOneAcc = vec3(0, 0, 0);

    return massOneAcc;
}

vec3 getAccelerationFromSpringConnection(uint massOne, uint massTwo) {
    if (massOne == BAD_INDEX || massTwo == BAD_INDEX) return vec3(0, 0, 0);
    vec3 originalPosition = Positions[massOne].xyz;
    vec3 originalVelocity = Velocities[massOne].xyz;
    float mass = MassParameters[massOne].mass;
    vec3 p2 = Positions[massTwo].xyz;
    vec3 v2 = Velocities[massTwo].xyz;

    vec3 a = getSpringAcceleration(originalPosition, originalVelocity, mass, p2, v2);
    vec3 v_half = originalVelocity + a * 0.5 * dt;
    vec3 p_half = originalPosition + v_half * 0.5 * dt;

    vec3 a_half = getSpringAcceleration(p_half, v_half, mass, p2, v2);
    return a_half;
}

void CalculateForces() {
    vec3 leftAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.left);
    vec3 rightAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.right);
    vec3 upAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.up);
    vec3 downAcc = getAccelerationFromSpringConnection(gid, MassParameters[gid].connections.down);

    vec3 acc = gravity + leftAcc + rightAcc + upAcc + downAcc;

    NewVelocities[gid].xyz += acc * dt;
}

void IntegrateForces() {
    Velocities[gid].xyz = NewVelocities[gid].xyz;

    if (!MassParameters[gid].isFixed) {
        Positions[gid].xyz += Velocities[gid].xyz * dt;
    } else {
        Velocities[gid].xyz = vec3(0, 0, 0);
    }
}

void ComputeNormals() {
    Connections connections = MassParameters[gid].connections;
    uint leftIndex = BAD_INDEX, upIndex = BAD_INDEX;

    // This makes me sad
    if (connections.left != BAD_INDEX && connections.up != BAD_INDEX) {
        leftIndex = connections.left;
        upIndex = connections.up;
    } else if (connections.up != BAD_INDEX && connections.right != BAD_INDEX) {
        leftIndex = connections.up;
        upIndex = connections.right;
    } else if (connections.right != BAD_INDEX && connections.down != BAD_INDEX) {
        leftIndex = connections.right;
        upIndex = connections.down;
    } else if (connections.down != BAD_INDEX && connections.left != BAD_INDEX) {
        leftIndex = connections.down;
        upIndex = connections.left;
    }

    if (leftIndex == BAD_INDEX || upIndex == BAD_INDEX) {
        Normals[gid].xyz = vec3(0, 0, 0);
        return;
    }

    vec3 toLeft = normalize(Positions[leftIndex].xyz - Positions[gid].xyz);
    vec3 toUp = normalize(Positions[upIndex].xyz - Positions[gid].xyz);

    vec3 normal = cross(toLeft, toUp);
    Normals[gid].xyz = normal;
}

void main() {
    gid = gl_GlobalInvocationID.x;

    if (computationStage == 0) {
        CalculateForces();
    } else if (computationStage == 1) {
        IntegrateForces();
        ComputeNormals();
    }
}