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

layout(std140, binding = 3) buffer Col {
    vec4 Colors[];
};

layout(std140, binding = 7) buffer ColMod {
    vec4 ColorMods[];
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
    int ParticleMode;
};

// layout(binding = 6, offset = 0) uniform atomic_uint NumDead;
layout(std430, binding = 6) buffer Atomics {
    int NumDead;
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

const float timestep = 0.01;
const float G = 50;
const float bounceFactor = -0.7;

const int FreeMode = 0;
const int MagicMode = 1;
const int WaterMode = 2;

uint gid = -1;
float randSeed = 1;

// -- Random Function -- //
// https://stackoverflow.com/a/28095165

const float PHI = 1.61803398874989484820459 * 00000.1;      // Golden Ratio
const float FAKE_PI = 3.14159265358979323846264 * 00000.1;  // PI
const float SQ2 = 1.41421356237309504880169 * 10000.0;      // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed) {
    return fract(tan(distance(coordinate * (seed + PHI), vec2(PHI, FAKE_PI))) * SQ2);
}

float gold_noise(float seed) {
    return gold_noise(vec2(seed + 1, seed + 2), seed);
}

// alternate
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(float n) {
    return fract(sin(n) * 43758.5453123);
}
// -- -- //

// -- Generate rotation matrix around axis -- //
// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0, 0.0, 0.0, 0.0, 1.0);
}

mat4 rotationMatrix(vec4 axis, float angle) {
    return rotationMatrix(axis.xyz, angle);
}
// -- -- //

void SetSpawnColor() {
    if (ParticleMode == WaterMode) {
        ColorMods[gid].r = ColorMods[gid].g = gold_noise(randSeed + 9) / 8.0;
        ColorMods[gid].b = -(gold_noise(randSeed + 10) / 10.0);
    } else { // pick a random color otherwise
        Colors[gid].r = rand(gid);
        Colors[gid].g = rand(gid + 1);
        Colors[gid].b = rand(gid + 2);
    }
}

void UpdateColor() {
    if (ParticleMode == WaterMode) {
        float heightCutoff = 35;
        float heightFoaminess = max((heightCutoff - min(Positions[gid].z, heightCutoff)) / heightCutoff, 0);

        float velCutoff = 20;
        float slownessBlueness = (velCutoff - min(length(Velocities[gid].xyz), velCutoff)) / velCutoff;

        float foaminess = max(heightFoaminess - slownessBlueness, 0);

        Colors[gid].xyz = vec3(foaminess, foaminess, 1) + ColorMods[gid].rgb;
    }

    if (Lifetimes[gid] < 0) {
        Colors[gid].a = 0;
    } else {
        Colors[gid].a = 1;
    }
}

// -- Spawning -- //
const float PI = 3.14159265358979323846264;
const vec4 diskCenter = vec4(20, 20, 50, 1);
const vec4 diskNormal = normalize(vec4(1, 0, 1, 1));
const float diskRadius = 10;
const float cylindricalHeight = -4;
const float launchAngle = -PI / 2;
const float launchVelocity = 30;
const vec4 up = vec4(0, 0, 1, 1);

void SpawnInDisk() {
    float r = diskRadius * sqrt(gold_noise(vec2(gid, gid), randSeed));
    float theta = rand(gid + randSeed + 1) * 2 * PI;

    vec4 vecInPlane = vec4(cross(up.xyz, diskNormal.xyz), 1);
    vec4 rotated = rotationMatrix(diskNormal.xyz, theta) * vecInPlane;

    float cylinderNoise = gold_noise(vec2(gid, gid), randSeed + 4);
    vec4 cylindricalOffset = cylinderNoise * diskNormal * cylindricalHeight;

    Positions[gid] = diskCenter + r * normalize(rotated) + cylindricalOffset;

    float noiseX = (rand(gid + randSeed + 102) - 0.5) * 4;
    float noiseY = (rand(gid + randSeed + 103) - 0.5) * 4;
    float noiseZ = (rand(gid + randSeed + 104) - 0.5) * 4;
    Velocities[gid] = vec4(noiseX, noiseY, noiseZ, 1) + diskNormal * launchVelocity;
}

// Adapted from https://karthikkaranth.me/blog/generating-random-points-in-a-sphere/
vec3 RandomPointInSphere(float sphereRadius, vec3 sphereCenter) {
    float u = rand(gid + randSeed + 500);
    float v = rand(gid + randSeed + 501);
    float theta = u * 2 * PI;
    float phi = acos(2 * v - 1);
    float r = pow(rand(gid + randSeed + 502), 1 / 3.0) * sphereRadius;
    float sinTheta = sin(theta);
    float cosTheta = cos(theta);
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    float x = r * sinPhi * cosTheta;
    float y = r * sinPhi * sinTheta;
    float z = r * cosPhi;

    return sphereCenter + vec3(x, y, z);
}

vec3 RandomPointInCube(float sideLength, vec3 center) {
    float x = (rand(gid + randSeed + 600) - 0.5) * sideLength;
    float y = (rand(gid + randSeed + 601) - 0.5) * sideLength;
    float z = (rand(gid + randSeed + 602) - 0.5) * sideLength;

    return vec3(x, y, z) + center;
}

vec3 RandomVelocity(float seed, float multiplier) {
    float noiseX = (rand(seed + 105) - 0.5);
    float noiseY = (rand(seed + 106) - 0.5);
    float noiseZ = (rand(seed + 107) - 0.5);

    return vec3(noiseX, noiseY, noiseZ) * multiplier;
}

void InitializeSpawnPositionAndVelocity() {
    if (ParticleMode == WaterMode) {
        SpawnInDisk();
    } else if (ParticleMode == FreeMode) {
        Positions[gid] = vec4(RandomPointInCube(100, vec3(50, 50, 50)), 1);
        Velocities[gid] = vec4(RandomVelocity(randSeed + gid, 4), 1);
    } else if (ParticleMode == MagicMode) {
        Positions[gid] = vec4(RandomPointInSphere(50, vec3(50, 50, 100)), 1);
    }
}

void Spawn() {
    Lifetimes[gid] = 1;
    // atomicAdd(NumDead, -1);

    SetSpawnColor();
    UpdateColor();
    InitializeSpawnPositionAndVelocity();
}

void Die() {
    Lifetimes[gid] = -1;
    Colors[gid].a = 0;
    Positions[gid] = vec4(10000, 10000, 10000, 1);
    // atomicAdd(NumDead, 1);
}
// -- -- //

void main() {
    gid = gl_GlobalInvocationID.x;
    randSeed = Time;
    float dt = timestep * SimulationSpeed;

    if (Lifetimes[gid] < 0) {
        if (ParticleMode == FreeMode || ParticleMode == MagicMode) {
            Spawn();
        } else if (ParticleMode == 2 && dt > 0 && gold_noise(vec2(gid, gid), randSeed + 3) < 0.000000000001) {
            Spawn();
        } else {
            return;
        }
    } else {
        Lifetimes[gid] += dt;
        if (length(Velocities[gid].xyz) < 1) {
            Lifetimes[gid] += 4 * dt;  // age still particles faster
        }
    }

    vec3 p = Positions[gid].xyz;
    vec3 v = Velocities[gid].xyz;
    float r = length(GravityCenter - Positions[gid].xyz) / 5;
    vec3 a = (normalize(GravityCenter - Positions[gid].xyz) * (G + (1 / pow(r, 2)))) * GravityFactor;
    if (ParticleMode == WaterMode) {
        a += vec3(0, 0, -9.86);
    }

    vec3 dta = dt * a;

    Positions[gid].xyz = p.xyz + v.xyz * dt + 0.5 * dt * dta;
    Velocities[gid].xyz = (v + dta) * 0.9999;

    UpdateColor();

    if (Positions[gid].z < minZ) {
        Positions[gid].z = minZ + 0.001;
        if (abs(Velocities[gid].z) > 10) {
            float theta = (rand(randSeed + gid + 21) - 0.5) * 0.6 * PI;
            float xyFactor = 1.0;
            float bounceFac = -1.0;
            if (rand(randSeed + gid + 20) < 0.2) {
                theta = PI + (rand(randSeed + gid + 24) - 0.5) * 1.4 * PI;
                xyFactor = 0.6;
                bounceFac = -0.6;
                if (rand(randSeed + gid + 25) < 0.4) {
                    theta = PI + (rand(randSeed + gid + 23) - 0.5) * 0.4 * PI;
                    xyFactor = 0.9;
                    bounceFac = -0.5;
                }
            }

            Velocities[gid].xy = (rotationMatrix(up, theta) * Velocities[gid]).xy;
            Velocities[gid].xy *= xyFactor;
            Velocities[gid].z *= bounceFac;
            Velocities[gid].z *= pow(rand(randSeed + gid + 22) - 0.11, 6);
            Velocities[gid].z += rand(randSeed + gid + 23);
        } else {
            Velocities[gid].z *= -0.25;
            Velocities[gid].xy *= 0.8;
        }
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

    float despawnTime;
    if (ParticleMode == WaterMode) {
        despawnTime = 15;
    } else {
        despawnTime = 100;
    }

    if (Lifetimes[gid] > despawnTime) {
        Die();
    }
}