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

struct massParams {
    GLboolean isFixed;
    GLfloat mass;
};

struct spring {
    GLint massOneIndex, massTwoIndex;
};

class ClothManager {
   public:
    ClothManager();

    void RenderParticles(float dt, Environment *environment);
    void InitGL();
    void UpdateComputeParameters(float dt);
    void ExecuteComputeShader() const;

    static const int MAX_NUM_SPRINGS = 128;
    static const int WORK_GROUP_SIZE = 32;

    static const int NUM_THREADS = 4;
    static const int MASSES_PER_THREAD = 8;
    static const int NUM_MASSES = NUM_THREADS * MASSES_PER_THREAD;
    static const int NUM_VERTICAL_SPRINGS = (MASSES_PER_THREAD - 1) * NUM_THREADS;
    static const int NUM_HORIZONTAL_SPRINGS = (NUM_THREADS - 1) * MASSES_PER_THREAD;
    static const int NUM_SPRINGS = NUM_VERTICAL_SPRINGS + NUM_HORIZONTAL_SPRINGS;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint newVelSSbo;
    static GLuint massSSbo;
    static GLuint springSSbo;
    static GLuint paramSSbo;

    simParams simParameters;
};
