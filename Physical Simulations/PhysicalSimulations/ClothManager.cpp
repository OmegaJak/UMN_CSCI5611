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
GLuint ClothManager::massSSbo;
GLuint ClothManager::paramSSbo;

ClothManager::ClothManager() {
    srand(time(NULL));
    simParameters = simParams{0, 0};
    InitGL();
}

void ClothManager::InitGL() {
    assert(NUM_MASSES % WORK_GROUP_SIZE == 0);

    // Prepare the positions buffer //
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(position), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass positions...\n");
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), bufMask);
    for (int i = 0; i < NUM_MASSES; i++) {
        int threadnum = i / MASSES_PER_THREAD;  // Deliberate int div for floor
        // positions[i] = {Utils::randBetween(0, 1), Utils::randBetween(0, 1) + threadnum * 3, 20, 0};
        float y = threadnum * 1 + Utils::randBetween(0, 1);
        float x = (i % MASSES_PER_THREAD) * 2 + Utils::randBetween(0, 1);
        positions[i] = {x, y, 20.0f, 1.0f};
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the mass parameters buffer //
    glGenBuffers(1, &massSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, massSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(massParams), nullptr, GL_STATIC_DRAW);

    printf("Initializing springs...\n");
    massParams *massParameters = (massParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(massParams), bufMask);
    for (int i = 0; i < NUM_MASSES; i++) {
        if (i % MASSES_PER_THREAD == 0) {
            massParameters[i].isFixed = true;
        } else {
            massParameters[i].isFixed = false;
        }
        massParameters[i].mass = 0.05;

        // Initialize connections
        unsigned int left = 95683, right = 95683, up = 95683, down = 95683;
        int threadnum = i / MASSES_PER_THREAD;
        int y = i % MASSES_PER_THREAD;
        if (threadnum < NUM_THREADS - 1) {
            left = i + MASSES_PER_THREAD;
        }

        if (threadnum > 0) {
            right = i - MASSES_PER_THREAD;
        }

        if (y > 0) {
            up = i - 1;
        }

        if (y < MASSES_PER_THREAD - 1) {
            down = i + 1;
        }

        massParameters[i].connections = {left, right, up, down};
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the velocities buffer
    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass velocities\n");
    velocity *vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    ////

    // Prepare the new velocities buffer
    glGenBuffers(1, &newVelSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newVelSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(velocity), nullptr, GL_STATIC_DRAW);

    vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(velocity), bufMask);
    memset(vels, 0, NUM_MASSES * sizeof(velocity));
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

void ClothManager::UpdateComputeParameters() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, paramSSbo);
    simParams *params = (simParams *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(simParams), bufMask);
    *params = simParameters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ClothManager::ExecuteComputeShader() {
    UpdateComputeParameters();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, paramSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, newVelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);

    glUseProgram(ShaderManager::ClothComputeShader);

    auto computationsPerFrame = int(((1 / IDEAL_FRAMERATE) / COMPUTE_SHADER_TIMESTEP) + 0.5);
    for (int i = 0; i < computationsPerFrame; i++) {
        simParameters.computationStage = 0;
        UpdateComputeParameters();
        glDispatchCompute(NUM_MASSES / WORK_GROUP_SIZE, 1, 1);  // Run the cloth sim compute shader
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);         // Wait for all to finish

        simParameters.computationStage = 1;
        UpdateComputeParameters();
        glDispatchCompute(NUM_MASSES / WORK_GROUP_SIZE, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    }

    int numSSbos = 6;
    for (int i = 0; i < numSSbos; i++) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 1, 0);
    }
}

void ClothManager::RenderParticles(float dt, Environment *environment) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), GL_MAP_READ_BIT);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (int i = 0; i < NUM_MASSES; i++) {
        environment->masses[i].SetPosition(glm::vec3(positions[i].x, positions[i].y, positions[i].z));
    }
}
