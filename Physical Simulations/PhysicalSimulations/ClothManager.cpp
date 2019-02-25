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
    srand(time(NULL));
    simParameters = simParams{0};
    InitGL();
}

void ClothManager::InitGL() {
    // Prepare the positions buffer //
    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NUM_SPRINGS * sizeof(position), nullptr, GL_STATIC_DRAW);

    printf("Initializing mass positions...\n");
    position *positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), bufMask);
    for (int i = 0; i < NUM_MASSES; i++) {
        int threadnum = i / MASSES_PER_THREAD;  // Deliberate int div for floor
        // positions[i] = {Utils::randBetween(0, 1), Utils::randBetween(0, 1) + threadnum * 5, 20, 0};
        float y = threadnum * 5;
        float x = (i % MASSES_PER_THREAD) * 5;
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
        massParameters[i].mass = 0.1;
    }

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
    int baseIndex = 0;
    for (int i = 0; i < NUM_VERTICAL_SPRINGS; i++, baseIndex++) {
        springs[i].massOneIndex = baseIndex;
        springs[i].massTwoIndex = baseIndex + 1;

        if ((i + 1) % (MASSES_PER_THREAD - 1) == 0 && i != 0) {
            baseIndex++;  // Don't link ends of threads
        }
    }
    for (int i = NUM_VERTICAL_SPRINGS; i < NUM_SPRINGS; i++) {
        int k = i - NUM_VERTICAL_SPRINGS;
        int threadIndex = k / MASSES_PER_THREAD;  // Int div for floor
        int offset = k % MASSES_PER_THREAD;

        springs[i].massOneIndex = threadIndex * MASSES_PER_THREAD + offset;
        springs[i].massTwoIndex = (threadIndex + 1) * MASSES_PER_THREAD + offset;
    }
    for (int i = NUM_SPRINGS; i < MAX_NUM_SPRINGS; i++) {
        springs[i].massOneIndex = -1;
        springs[i].massTwoIndex = -1;
    }

    for (int i = 0; i < NUM_SPRINGS; i++) {
        printf("Spring %i: (%i, %i)\n", i, springs[i].massOneIndex, springs[i].massTwoIndex);
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
    assert(NUM_MASSES % WORK_GROUP_SIZE == 0);
    assert(MAX_NUM_SPRINGS % WORK_GROUP_SIZE == 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, springSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, paramSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, newVelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);

    auto computationsPerFrame = int(((1 / IDEAL_FRAMERATE) / COMPUTE_SHADER_TIMESTEP) + 0.5);
    for (int i = 0; i < computationsPerFrame; i++) {
        glUseProgram(ShaderManager::ClothComputeShader);
        glDispatchCompute(1, 1, 1);                      // Run the cloth sim compute shader
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // Wait for all to finish

        // Copy the temp vel values to the velocities, run integration to update positions
        glUseProgram(ShaderManager::IntegratorComputeShader);
        glDispatchCompute(NUM_MASSES / WORK_GROUP_SIZE, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, springSSbo);
    spring *springs = (spring *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRINGS * sizeof(spring), GL_MAP_READ_BIT);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (int i = 0; i < NUM_SPRINGS; i++) {
        auto massOne = springs[i].massOneIndex;
        auto massTwo = springs[i].massTwoIndex;

        environment->springs[i].SetPosition(glm::vec3((positions[massOne].x + positions[massTwo].x) / 2,
                                                      (positions[massOne].y + positions[massTwo].y) / 2,
                                                      (positions[massOne].z + positions[massTwo].z) / 2));
    }
}
