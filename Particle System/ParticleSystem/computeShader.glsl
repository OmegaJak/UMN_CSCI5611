#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std140, binding = 1) buffer Pos {
    vec4 Positions[];
};

layout(std140, binding = 2) buffer Vel {
    vec4 Velocities[];
};

layout(std140, binding = 3) buffer Col {
    vec4 Colors[];
};

layout(std430, binding = 5) buffer Life {
    float Lifetimes[];
};

layout(std430, binding = 4) buffer Parameters {
    vec3 GravityCenter;
    float minX, minY, minZ;
    float maxX, maxY, maxZ;
    float SimulationSpeed;
    float GravityFactor;
    float SpawnRate;
    float Time;
};

//layout(binding = 6, offset = 0) uniform atomic_uint NumDead;
layout(std430, binding = 6) buffer Atomics {
    int NumDead;
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

const float timestep = 0.01;
const float G = 50;
const float bounceFactor = -0.7;

// -- Random Function -- //
// https://stackoverflow.com/a/28095165

float PHI = 1.61803398874989484820459 * 00000.1;  // Golden Ratio
float PI = 3.14159265358979323846264 * 00000.1;   // PI
float SQ2 = 1.41421356237309504880169 * 10000.0;  // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed) {
    return fract(tan(distance(coordinate * (seed + PHI), vec2(PHI, PI))) * SQ2);
}
// -- End Random Function -- //

void main() {
    uint gid = gl_GlobalInvocationID.x;
    float dt = timestep * SimulationSpeed;
    // const uvec3 idx = gl_WorkGroupID* gl_WorkGroupSize + gl_LocalInvocationID;

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;
    float r = length(GravityCenter - Positions[gid].xyz) / 5;
    vec3 a = (normalize(GravityCenter - Positions[gid].xyz) * (G + (1 / pow(r, 2)))) * GravityFactor;

    vec3 dta = dt * a;

    Positions[gid].xyz = p.xyz + v.xyz * dt + 0.5 * dt * dta;
    Velocities[gid].xyz = (v + dta) * 0.9999;

    if (Positions[gid].z < minZ) {
        Positions[gid].z = minZ;
        Velocities[gid].z *= bounceFactor;
    }
    if (Positions[gid].z > maxZ) {
        Positions[gid].z = maxZ;
        Velocities[gid].z *= bounceFactor;
    }
    if (Positions[gid].x < minX) {
        Positions[gid].x = minX;
        Velocities[gid].x *= bounceFactor;
    }
    if (Positions[gid].x > maxX) {
        Positions[gid].x = maxX;
        Velocities[gid].x *= bounceFactor;
    }
    if (Positions[gid].y < minY) {
        Positions[gid].y = minY;
        Velocities[gid].y *= bounceFactor;
    }
    if (Positions[gid].y > maxY) {
        Positions[gid].y = maxY;
        Velocities[gid].y *= bounceFactor;
    }

    //if (Lifetimes[gid] > 600) {
    //    Lifetimes[gid] = -1;
    //    Colors[gid].a = 0;
    //    atomicAdd(NumDead, 1);
    //}

    //if (Lifetimes[gid] < 0) {
    //    float spawnChance = SpawnRate / float(NumDead);
    //    
    //    if (gold_noise(vec2(gid, gid), Time) < spawnChance) {
    //        Lifetimes[gid] = 1;
    //        atomicAdd(NumDead, -1);
    //        Colors[gid].a = 1;
    //    }
    //    /*float val = gold_noise(vec2(gid, gid), Time);
    //    Colors[gid] = vec4(val, val, val, 1);
    //    Positions[gid].z = Time;*/
    //} else {
    //    Lifetimes[gid] += 1;
    //}
}