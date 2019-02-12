#pragma once
#include "Model.h"
#include "glad.h"

struct particleParams {
    GLfloat centerX, centerY, centerZ;
    GLfloat minX, minY, minZ;
    GLfloat maxX, maxY, maxZ;
    GLfloat simulationSpeed;
    GLfloat gravityAccelerationFactor;
    GLfloat spawnRate;
    GLfloat time;
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

class ParticleManager {
   public:
    ParticleManager();

    void RenderParticles(float dt);
    void InitGL();
    int GetNumParticles();
    void UpdateComputeParameters();

    float genRate = 1000;

    static const int NUM_PARTICLES = 16 * 1024 * 1024;
    static const int WORK_GROUP_SIZE = 128;

    static int numAlive;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint colSSbo;
    static GLuint colModSSbo;
    static GLuint lifeSSbo;
    static GLuint paramSSbo;
    static GLuint atomicsSSbo;

    particleParams particleParameters;
};
