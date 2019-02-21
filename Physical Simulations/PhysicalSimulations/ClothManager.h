#pragma once
#include "Model.h"
#include "glad.h"

class Environment;

struct simParams {
    GLint idk;
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
    static const int WORK_GROUP_SIZE = 64;

    static const int NUM_MASSES = 2;
    static const int NUM_SPRINGS = 1;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint newVelSSbo;
    static GLuint massSSbo;
    static GLuint springSSbo;
    static GLuint paramSSbo;

    simParams simParameters;
};
