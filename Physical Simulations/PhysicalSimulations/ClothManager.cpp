#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>
#include <ctime>
#include "ClothManager.h"
#include "Constants.h"
#include "Environment.h"
#include "ShaderManager.h"
#include "Utils.h"
#include "glad.h"

const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

GLuint ClothManager::posSSbo;
GLuint ClothManager::velSSbo;
GLuint ClothManager::newVelSSbo;
GLuint ClothManager::springSSbo;
GLuint ClothManager::massSSbo;
GLuint ClothManager::paramSSbo;

ClothManager::ClothManager() {
    simParameters = simParams{1};
    InitGL();
}

void ClothManager::InitGL() {
    // Prepare the positions buffer //
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(position), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass positions...\n");
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), bufMask);
    positions[0] = {0, 0, 10, 0};
    positions[1] = {0, 0, 5, 0};
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the mass parameters buffer //
    glGenBuffers(1, &massSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(spring), nullptr, GL_STATIC_DRAW);

    printf("Initializing springs...\n");
    massParams *massParameters = (massParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAX_NUM_SPRINGS * sizeof(spring), bufMask);
    massParameters[0].isFixed = true;
    massParameters[0].mass = 10;
    massParameters[1].isFixed = false;
    massParameters[1].mass = 0.5;

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass velocities\n");
    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the new velocities buffer
    glGenBuffers(1, &newVelSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newVelSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the springs buffer //
    glGenBuffers(1, &springSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, springSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(spring), nullptr, GL_STATIC_DRAW);

    printf("Initializing springs...\n");
    spring *springs = (spring *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, MAX_NUM_SPRINGS * sizeof(spring), bufMask);

    // Initialize springs
    int numSprings = 1;
    springs[0].massOneIndex = 0;
    springs[0].massTwoIndex = 1;
    for (int i = numSprings; i < MAX_NUM_SPRINGS; i++) {
        springs[i].massOneIndex = -1;
        springs[i].massTwoIndex = -1;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Misc data //
    glGenBuffers(1, &paramSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(simParams), nullptr, GL_STATIC_DRAW);

    simParams *params = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *params = simParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    printf("Done initializing buffers\n");
}

void ClothManager::UpdateComputeParameters(float dt) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    simParams *params = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *params = simParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::ExecuteComputeShader() const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, springSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, paramSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, newVelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);

    glUseProgram(ShaderManager::ClothComputeShader);

    auto computationsPerFrame = int(((1 / IDEAL_FRAMERATE) / COMPUTE_SHADER_TIMESTEP) + 0.5);
    for (int i = 0; i < computationsPerFrame; i++) {
        glDispatchCompute(MAX_NUM_SPRINGS / WORK_GROUP_SIZE, 1, 1);  // Compute shader!!
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    // Copy the new velocity values to the velocities buffer
    // glCopyBufferSubData(newVelSSbo, velSSbo, 0, 0, NUM_MASSES * sizeof(velocity));

    int numSSbos = 6;
    for (int i = 0; i < numSSbos; i++) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 1, 0);
    }
}

void ClothManager::RenderParticles(float dt, Environment *environment) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), GL_MAP_READ_BIT);

    for (int i = 0; i < NUM_MASSES; i++) {
        environment->masses[i].SetPosition(glm::vec3(positions[i].x, positions[i].y, positions[i].z));
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
