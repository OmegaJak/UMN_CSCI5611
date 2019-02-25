#pragma once
#include "Model.h"
#include "glad.h"

class Environment;

struct simParams {
    GLfloat dt;
};

struct position {
    GLfloat x, y, z, w;
};

struct velocity {
    GLfloat vx, vy, vz, vw;
};

struct massConnections {
    GLuint left, right, up, down;
};

struct massParams {
    GLboolean isFixed;
    GLfloat mass;
    massConnections connections;
};

class ClothManager {
   public:
    ClothManager();

    void RenderParticles(float dt, Environment *environment);
    void InitGL();
    void UpdateComputeParameters() const;
    void ExecuteComputeShader();

    static const int WORK_GROUP_SIZE = 32;

    static const int NUM_THREADS = 32;
    static const int MASSES_PER_THREAD = 16;
    static const int NUM_MASSES = NUM_THREADS * MASSES_PER_THREAD;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint newVelSSbo;
    static GLuint massSSbo;
    static GLuint paramSSbo;

    simParams simParameters;
};
