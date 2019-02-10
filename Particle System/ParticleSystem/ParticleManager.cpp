#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>
#include <ctime>
#include <gtc/type_ptr.hpp>
#include "Constants.h"
#include "ParticleManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "glad.h"

const float radius = 0.5;
const float bounceFactor = -0.8;
const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

ParticleManager::ParticleManager() {
    _particleModel = new Model("models/sphere.txt");
    srand(time(NULL));
    particleParameters = particleParams{
        50.f,    50.f,    50.f,     // gravity center
        -5000.f, -5000.f, -5000.f,  // min
        5000.f,  5000.f,  5000.f,   // max
        1.0f,                       // sim speed
        0.0f,                       // grav factor
    };
    InitGL();
}

void ParticleManager::InitGL() {
    // Prepare the positions buffer
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(position), NULL, GL_STATIC_DRAW);

    position *points = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(position), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        points[i].x = randBetween(0, 100);
        points[i].y = randBetween(0, 100);
        points[i].z = randBetween(0, 100);
        points[i].w = 1;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(velocity), NULL, GL_STATIC_DRAW);

    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(velocity), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        vels[i].vx = randBetween(0, 15) - 7.5;
        vels[i].vy = randBetween(0, 15) - 7.5;
        vels[i].vz = randBetween(0, 15) - 7.5;
        vels[i].vw = 1;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Prepare the colors buffer
    glGenBuffers(1, &colSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(color), NULL, GL_STATIC_DRAW);

    color *colors = (color *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(color), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        colors[i].r = randBetween(0, 1);
        colors[i].g = randBetween(0, 1);
        colors[i].b = randBetween(0, 1);
        colors[i].a = 1;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Misc data
    glGenBuffers(1, &paramSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleParams), NULL, GL_STATIC_DRAW);

    particleParams *params = (particleParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(particleParams), bufMask);
    *params = particleParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

int ParticleManager::GetNumParticles() {
    return NUM_PARTICLES;
}

void ParticleManager::RenderParticles(float dt) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    particleParams *params = (particleParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(particleParams), bufMask);
    *params = particleParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glUseProgram(ShaderManager::Particle_Render_Shader);

    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float ParticleManager::randBetween(int min, int max) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

GLuint ParticleManager::posSSbo;
GLuint ParticleManager::velSSbo;
GLuint ParticleManager::colSSbo;
GLuint ParticleManager::paramSSbo;
