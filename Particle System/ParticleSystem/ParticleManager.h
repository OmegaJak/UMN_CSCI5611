#pragma once
#include "Model.h"
#include "glad.h"

struct particleParams {
    GLfloat centerX, centerY, centerZ;
    GLfloat minX, minY, minZ;
    GLfloat maxX, maxY, maxZ;
    GLfloat simulationSpeed;
    GLfloat gravityAccelerationFactor;
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

class ParticleManager {
   public:
    ParticleManager();

    void RenderParticles(float dt);
    void InitGL();
    int GetNumParticles();

    float genRate = 1000;

    static const int NUM_PARTICLES = 2 * 1024 * 1024;
    static const int WORK_GROUP_SIZE = 128;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint colSSbo;
    static GLuint paramSSbo;

    particleParams particleParameters;

   private:
    static float randBetween(int min, int max);

    Model* _particleModel;
};
