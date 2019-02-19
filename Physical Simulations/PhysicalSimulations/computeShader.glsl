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
    float PlayerX, PlayerY, PlayerZ;
    float SimulationSpeed;
    float GravityFactor;
    float SpawnRate;
    float Time;
    int ParticleMode;
    int FireballState;
};

// layout(binding = 6, offset = 0) uniform atomic_uint NumDead;
layout(std430, binding = 6) buffer Atomics {
    int NumDead;
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

const float timestep = 0.01;
const float G = 50;
const float bounceFactor = -0.7;

// -- Poor man's enums -- //
const int FreeMode = 0;
const int FireballMode = 1;
const int WaterMode = 2;

const int Waiting = 0;
const int Spawning = 1;
const int Moving = 2;
const int Exploding = 3;
// -- -- //

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

vec3 RandomVelocity(float seed, float multiplier) {
    float noiseX = (rand(seed + 105) - 0.5);
    float noiseY = (rand(seed + 106) - 0.5);
    float noiseZ = (rand(seed + 107) - 0.5);

    return vec3(noiseX, noiseY, noiseZ) * multiplier;
}

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
const float startingTemperature = 10;
const float outerHeatThreshold = 50;
const float coreHeatThreshold = 10;

void SetSpawnColor() {
    if (ParticleMode == WaterMode) {
        ColorMods[gid].r = ColorMods[gid].g = gold_noise(randSeed + 9) / 8.0;
        ColorMods[gid].b = -(gold_noise(randSeed + 10) / 10.0);
    } else if (ParticleMode == FreeMode) { 
        Colors[gid].r = rand(gid);
        Colors[gid].g = rand(gid + 1);
        Colors[gid].b = rand(gid + 2);
    } else if (ParticleMode == FireballMode) {
        float randDarkness = rand(randSeed + gid + 57) * 0.3;
        Colors[gid] = vec4(1 - randDarkness, 0, 0, 1);
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
    } else if (ParticleMode == FireballMode) {
        if (FireballState == Moving) {
            vec3 toPlayer = normalize(vec3(PlayerX, PlayerY, PlayerZ) - Positions[gid].xyz);
            vec3 posOnSphere = normalize(Positions[gid].xyz - GravityCenter);
            float angle = dot(toPlayer, posOnSphere);
            angle = max(angle, 0.0001); // clamp to zero below threshold to avoid bad edge behavior

            Colors[gid].xyz = vec3(1, angle, pow(angle, 10));
        } else if (FireballState == Exploding || FireballState == Waiting) {
            float thresh = coreHeatThreshold;
            float distFromCenter = length(Positions[gid].xyz - GravityCenter);
            float closenessToCenter = max((thresh - distFromCenter) / thresh, 0);
            float r = 1;
            if (closenessToCenter == 0) {
                r = max(r - (distFromCenter - thresh) / outerHeatThreshold, 0.1);
            }

            // 0 to 1, increases as particle cools
            //float coolness = (startingTemperature - ColorMods[gid].a) / startingTemperature;
            float heat = pow(ColorMods[gid].a / startingTemperature, 2);
            
            Colors[gid].rgb = vec3(r, closenessToCenter, closenessToCenter / 4);
            Colors[gid].rgb -= ColorMods[gid].rgb;
            Colors[gid].rgb *= vec3(heat, heat, heat);
        }
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

void InitializeSpawnPositionAndVelocity() {
    if (ParticleMode == WaterMode) {
        SpawnInDisk();
    } else if (ParticleMode == FreeMode) {
        Positions[gid] = vec4(RandomPointInCube(100, vec3(50, 50, 50)), 1);
        Velocities[gid] = vec4(RandomVelocity(randSeed + gid, 4), 1);
    } else if (ParticleMode == FireballMode && FireballState == Spawning) {
        Positions[gid] = vec4(RandomPointInSphere(3, vec3(GravityCenter.x, GravityCenter.y, GravityCenter.z)), 1);
        Velocities[gid] = vec4(Positions[gid].xyz - GravityCenter, 1); // Store the position relative to the center in velocity to maintain constant relative position
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

    if (ParticleMode == FireballMode) {
        if (FireballState == Spawning) {
            Spawn();
        } else if (FireballState == Exploding) {
            /*Colors[gid] = vec4(1, 1, 0, 1);*/

            vec3 vel = RandomPointInSphere(pow(length(Velocities[gid].xyz), 2) * 3, vec3(0, 0, 0));
            vel.z = abs(vel.z);
            Velocities[gid] = vec4(vel, 1);

            vec3 randColor = RandomVelocity(gid + randSeed, 0.15);
            randColor.r /= 2;
            ColorMods[gid].rgb = randColor;
            ColorMods[gid].a = startingTemperature; // Use the alpha channel of ColorMods to store the 'temperature' of the particle
        }
    }

    if (Lifetimes[gid] < 0) {
        if (ParticleMode == FreeMode) {
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

    if (ParticleMode == FireballMode && FireballState == Moving) {
        Positions[gid].xyz = GravityCenter + Velocities[gid].xyz;
    } else if (ParticleMode != FireballMode || FireballState != Spawning) {
        vec3 p = Positions[gid].xyz;
        vec3 v = Velocities[gid].xyz;
        float r = length(GravityCenter - Positions[gid].xyz) / 5;
        vec3 a = (normalize(GravityCenter - Positions[gid].xyz) * (G + (1 / pow(r, 2)))) * GravityFactor;
        if (ParticleMode != FreeMode) {
            a += vec3(0, 0, -9.86);
        }

        vec3 dta = dt * a;

        Positions[gid].xyz = p.xyz + v.xyz * dt + 0.5 * dt * dta;
        Velocities[gid].xyz = (v + dta) * 0.9999;
    }

    UpdateColor();

    if (ParticleMode == FireballMode && (FireballState == Exploding || FireballState == Waiting)) {
        float distFromCenter = length(Positions[gid].xyz - GravityCenter);
        ColorMods[gid].a -= dt * max(distFromCenter / 50, 0.8);

        ColorMods[gid].a = max(ColorMods[gid].a, 0);
    }

    if (ParticleMode == WaterMode && length(Positions[gid].xyz - GravityCenter) < 4.75) {
        vec3 toParticle = normalize(Positions[gid].xyz - GravityCenter);
        //Positions[gid].xyz = Positions[gid].xyz + toParticle * 5;
        Velocities[gid].xyz = toParticle * max(length(Velocities[gid].xyz) * 0.5, 4.75);
    }

    if (Positions[gid].z < minZ && !(ParticleMode == FireballMode && FireballState == Spawning)) {
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

    if (ParticleMode == FireballMode && (FireballState == Exploding || FireballState == Waiting) && (ColorMods[gid].a == 0 || length(Colors[gid].rgb) < 0.05)) {
        Die();    
    }
}