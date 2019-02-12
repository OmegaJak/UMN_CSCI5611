#pragma once
#include "Model.h"
#include "glad.h"

struct particleParams {
    GLfloat centerX, centerY, centerZ;
    GLfloat minX, minY, minZ;
    GLfloat maxX, maxY, maxZ;
    GLfloat playerX, playerY, playerZ;
    GLfloat simulationSpeed;
    GLfloat gravityAccelerationFactor;
    GLfloat spawnRate;
    GLfloat time;
    GLint particleMode;
    GLint fireballState;  // 0 = waiting to spawn, 1 = spawning, 2 = spawned and moving, 3 = exploding
};

struct position {
    GLfloat x, y, z, w;
};

struct velocity {
    GLfloat vx, vy, vz, vw;
};

struct color {
    float r, g, b, a;
};

struct atomics {
    GLint numDead;
};

enum ParticleMode { Free_Mode = 0, Fireball_Mode = 1, Water_Mode = 2 };

class ParticleManager {
   public:
    ParticleManager();

    void RenderParticles(float dt);
    void InitGL();
    int GetNumParticles();
    void UpdateComputeParameters(float dt);
    void SpawnFireball(const glm::vec3& position, const glm::vec3& velocity);

    float genRate = 1000;

    static const int NUM_PARTICLES = 8 * 1024 * 1024;
    static const int WORK_GROUP_SIZE = 128;

    static int numAlive;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint colSSbo;
    static GLuint colModSSbo;
    static GLuint lifeSSbo;
    static GLuint paramSSbo;
    static GLuint atomicsSSbo;

    static ParticleMode PARTICLE_MODE;
    // 0 = zero-g original sim
    // 1 = fireball
    // 2 = waterfall

    particleParams particleParameters;

    glm::vec3 fireballPositions[1];
    glm::vec3 fireballVelocities[1];
    bool fireballAlive[1];

   private:
    void UpdateFireball(float dt);

    int computesSinceFireballEvent = 0;
};
