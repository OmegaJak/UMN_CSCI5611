#pragma once
#include "Model.h"
#include "glad.h"

struct particleParams {
    GLfloat centerX, centerY, centerZ;
    GLfloat simulationSpeed;
    GLfloat gravityAccelerationFactor;
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

    GLuint posSSbo;
    GLuint velSSbo;
    GLuint colSSbo;
    GLuint paramSSbo;

    particleParams particleParameters;

   private:
    static float randBetween(int min, int max);

    Model* _particleModel;
};
