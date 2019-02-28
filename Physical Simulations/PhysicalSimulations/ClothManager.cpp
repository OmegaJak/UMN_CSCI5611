#define GLM_FORCE_RADIANS

#include <SDL_stdinc.h>
#include <ctime>
#include <gtc/type_ptr.hpp>
#include "ClothManager.h"
#include "Constants.h"
#include "Environment.h"
#include "ShaderManager.h"
#include "Utils.h"
#include "glad.h"

const GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

GLuint ClothManager::posSSbo;
GLuint ClothManager::velSSbo;
GLuint ClothManager::normSSbo;
GLuint ClothManager::newVelSSbo;
GLuint ClothManager::massSSbo;
GLuint ClothManager::paramSSbo;
GLuint ClothManager::lastPosSSbo;

ClothManager::ClothManager() {
    srand(time(NULL));
    simParameters = simParams{0, 0, 0, 4, 0, 150, 30, 0.45 * CLOTH_HEIGHT / float(MASSES_PER_THREAD)};
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
        float y = threadnum * 0.5 * (CLOTH_WIDTH / float(NUM_THREADS));
        float x = (i % MASSES_PER_THREAD) * simParameters.restLength;
        if (i % MASSES_PER_THREAD != 0) {
            y += (Utils::randBetween(0, 1) - 0.5) * simParameters.restLength * 0.5;
            x += (Utils::randBetween(0, 1) - 0.5) * simParameters.restLength * 0.5;
        }
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
        massParameters[i].isFixed = i % MASSES_PER_THREAD == 0;
        // massParameters[i].isFixed = false;
        massParameters[i].mass = float(CLOTH_WEIGHT) / float(NUM_MASSES);

        // Initialize connections
        unsigned int left = BAD_INDEX, right = BAD_INDEX, up = BAD_INDEX, down = BAD_INDEX;
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

    // Prepare the normals buffer
    glGenBuffers(1, &normSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, normSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(normal), nullptr, GL_STATIC_DRAW);

    printf("Initializing cloth normals\n");
    normal *normals = (normal *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(normal), bufMask);
    memset(normals, 0, NUM_MASSES * sizeof(normal));
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

    // Prepare the last positions buffer
    glGenBuffers(1, &lastPosSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lastPosSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_MASSES * sizeof(position), nullptr, GL_STATIC_DRAW);

    positions = (position *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_MASSES * sizeof(position), bufMask);
    memset(positions, 0, NUM_MASSES * sizeof(position));
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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, normSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, paramSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, newVelSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, massSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, lastPosSSbo);

    glUseProgram(ShaderManager::ClothComputeShader);

    for (int i = 0; i < COMPUTES_PER_FRAME; i++) {
        glUniform1i(ShaderManager::ClothComputeStage, 0);
        glDispatchCompute(NUM_MASSES / WORK_GROUP_SIZE, 1, 1);  // Run the cloth sim compute shader
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);         // Wait for all to finish

        glUniform1i(ShaderManager::ClothComputeStage, 1);
        glDispatchCompute(NUM_MASSES / WORK_GROUP_SIZE, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BUFFER);
    }

    int numSSbos = 7;
    for (int i = 0; i < numSSbos; i++) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i + 1, 0);
    }
}

void ClothManager::InitClothIBO() {
    assert(NUM_TRIANGLE_INDICES % 3 == 0);
    assert(TRIANGLES_PER_THREAD % 2 == 0);

    glGenBuffers(1, &ShaderManager::ClothShader.IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ShaderManager::ClothShader.IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NUM_TRIANGLE_INDICES * sizeof(GLuint), nullptr, GL_STATIC_DRAW);
    auto indices = (GLuint *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, NUM_TRIANGLE_INDICES * sizeof(GLuint), bufMask);

    glm::uvec3 evenBaseTriangleIndices = {0, 1, MASSES_PER_THREAD};
    glm::uvec3 oddBaseTriangleIndices = {1, MASSES_PER_THREAD + 1, MASSES_PER_THREAD};
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        int y = (i / 2) % (TRIANGLES_PER_THREAD / 2);
        int threadnum = i / TRIANGLES_PER_THREAD;
        glm::uvec3 baseIndices;
        if (i % 2 == 0) {
            baseIndices = evenBaseTriangleIndices;
        } else {
            baseIndices = oddBaseTriangleIndices;
        }

        baseIndices += glm::uvec3(MASSES_PER_THREAD * threadnum);
        glm::uvec3 triangleIndices = baseIndices + glm::uvec3(y);

        int indexBaseIndex = i * 3;
        assert(indexBaseIndex < NUM_TRIANGLE_INDICES);

        indices[indexBaseIndex] = triangleIndices[0];
        indices[indexBaseIndex + 1] = triangleIndices[1];
        indices[indexBaseIndex + 2] = triangleIndices[2];
    }

    /*for (int i = 0; i < NUM_TRIANGLE_INDICES; i++) {
        printf("%u, ", indices[i]);

        if (i % 3 == 2) printf("\n");
    }*/

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void ClothManager::RenderParticles(float dt, Environment *environment) {
    glBindVertexArray(ShaderManager::ClothShader.VAO);

    auto model = glm::mat4();
    glUniformMatrix4fv(ShaderManager::ClothShader.Attributes.model, 1, GL_FALSE, glm::value_ptr(model));

    auto color = glm::vec3(101 / 255.0, 67 / 255.0, 33 / 255.0);
    glUniform3fv(ShaderManager::ClothShader.Attributes.color, 1, glm::value_ptr(color));  // Update the color, if necessary
    glUniform1i(ShaderManager::ClothShader.Attributes.texID, UNTEXTURED);                 // Set which texture to use
    glUniform1f(ShaderManager::EnvironmentShader.Attributes.specFactor, 0.2);

    glDrawElements(GL_TRIANGLES, NUM_TRIANGLE_INDICES, GL_UNSIGNED_INT, (void *)0);

    glBindVertexArray(ShaderManager::EnvironmentShader.VAO);
}
