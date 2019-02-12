#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>
#include <ctime>
#include "Constants.h"
#include "ParticleManager.h"
#include "ShaderManager.h"
#include "Utils.h"
#include "glad.h"

const float radius = 0.5;
const float bounceFactor = -0.8;
const int numAtomicCounters = 1;
const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

GLuint ParticleManager::posSSbo;
GLuint ParticleManager::velSSbo;
GLuint ParticleManager::colSSbo;
GLuint ParticleManager::colModSSbo;
GLuint ParticleManager::lifeSSbo;
GLuint ParticleManager::paramSSbo;
GLuint ParticleManager::atomicsSSbo;

int ParticleManager::numAlive;
ParticleMode ParticleManager::PARTICLE_MODE;

ParticleManager::ParticleManager() {
    srand(time(NULL));
    particleParameters = particleParams{
        50.f,         50.f,    50.f,    // gravity center
        -5000.f,      -5000.f, 0.f,     // min
        5000.f,       5000.f,  5000.f,  // max
        1.0f,                           // sim speed
        0.0f,                           // grav factor
        0.1f,                           // spawn rate - particles per second
        1.f,                            // Time
        PARTICLE_MODE                   // Which particle sim to do
    };
    if (PARTICLE_MODE == Free_Mode) {
        particleParameters.minZ = -5000.f;
    }

    numAlive = 0;
    InitGL();
}

void ParticleManager::InitGL() {
    // Prepare the positions buffer
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(position), nullptr, GL_STATIC_DRAW);

    printf("Initializing Particle spawn positions...\n");
    position *points = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(position), bufMask);
    memset(points, 0, NUM_PARTICLES * sizeof(position));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    printf("Initializing particle spawn velocities\n");
    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(velocity), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        /*vels[i].vx = randBetween(0, 15) - 7.5;
        vels[i].vy = randBetween(0, 15) - 7.5;
        vels[i].vz = randBetween(0, 15) - 7.5;*/

        vels[i].vx = 0;
        vels[i].vy = 0;
        vels[i].vz = 0;
        vels[i].vw = 1;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the colors buffer
    glGenBuffers(1, &colSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(color), nullptr, GL_STATIC_DRAW);

    printf("Initializing particle spawn colors...\n");
    color *colors = (color *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(color), bufMask);
    memset(colors, 0, NUM_PARTICLES * sizeof(color));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the color mods buffer
    glGenBuffers(1, &colModSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colModSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(color), nullptr, GL_STATIC_DRAW);

    printf("Initializing particle spawn color mods...\n");
    color *colorMods = (color *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(color), bufMask);
    memset(colorMods, 0, NUM_PARTICLES * sizeof(color));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the lifetimes buffer
    glGenBuffers(1, &lifeSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lifeSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);

    // Initialize lifetimes to zero
    // The lifetimes buffer is all floats, single value each. One value each = R, with 32-bit floats, so GL_R32F. We're setting the
    // 'red' bit (GL_RED) here of each to the initial value
    float startingLifetime = -1;
    glClearBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_R32F, 0, NUM_PARTICLES * sizeof(GLfloat), GL_RED, GL_FLOAT, &startingLifetime);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Misc data
    glGenBuffers(1, &paramSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleParams), nullptr, GL_STATIC_DRAW);

    particleParams *params = (particleParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(particleParams), bufMask);
    *params = particleParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the atomics buffer
    glGenBuffers(1, &atomicsSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicsSSbo);

    atomics initialAtomics = {NUM_PARTICLES};
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(atomicsSSbo), &initialAtomics, GL_STATIC_DRAW);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    printf("Done initializing particle buffers\n");
}

int ParticleManager::GetNumParticles() {
    return NUM_PARTICLES;
}

void ParticleManager::UpdateComputeParameters() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    particleParams *params = (particleParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(particleParams), bufMask);
    *params = particleParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicsSSbo);
    atomics *currentAtomics = (atomics *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(atomics), GL_MAP_READ_BIT);
    numAlive = NUM_PARTICLES - currentAtomics->numDead;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ParticleManager::RenderParticles(float dt) {
    /*glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomicsSSbo);
    atomics *currentAtomics = (atomics *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(atomics), GL_MAP_READ_BIT);
    printf("%f, %i\n", particleParameters.spawnRate / (float)currentAtomics->numDead, currentAtomics->numDead);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/

    glUseProgram(ShaderManager::ParticleShader.Program);

    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
