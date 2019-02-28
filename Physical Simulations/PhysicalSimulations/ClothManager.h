#pragma once
#include "Model.h"
#include "glad.h"

class Environment;

struct simParams {
    GLfloat obstacleCenterX, obstacleCenterY, obstacleCenterZ;
    GLfloat obstacleRadius;
    GLfloat dt;
    GLfloat ks;
    GLfloat kd;
    GLfloat restLength;
};

struct position {
    GLfloat x, y, z, w;
};

struct velocity {
    GLfloat vx, vy, vz, vw;
};

struct normal {
    GLfloat nx, ny, nz, nw;
};

struct massConnections {
    GLuint left, right, up, down;
};

struct massParams {
    GLboolean isFixed;
    GLfloat mass;
    massConnections connections;
};

struct texcoord {
    GLfloat u, v;
};

class ClothManager {
   public:
    ClothManager();

    void RenderParticles(float dt, Environment *environment);
    void InitGL();
    void UpdateComputeParameters() const;
    void ExecuteComputeShader();
    static void InitClothTexcoords();
    static void InitClothIBO();

    static const int WORK_GROUP_SIZE = 128;

    static const int NUM_THREADS = 128;
    static const int MASSES_PER_THREAD = 180;
    static const int NUM_MASSES = NUM_THREADS * MASSES_PER_THREAD;

    static const int CLOTH_WIDTH = 32;
    static const int CLOTH_HEIGHT = 32;
    static const int CLOTH_WEIGHT = 25;

    static const int TRIANGLES_PER_THREAD = (2 * (MASSES_PER_THREAD - 1));
    static const int NUM_TRIANGLES = TRIANGLES_PER_THREAD * (NUM_THREADS - 1);
    static const int NUM_TRIANGLE_INDICES = NUM_TRIANGLES * 3;

    static GLuint posSSbo;
    static GLuint velSSbo;
    static GLuint normSSbo;
    static GLuint newVelSSbo;
    static GLuint massSSbo;
    static GLuint paramSSbo;
    static GLuint lastPosSSbo;

    simParams simParameters;
};
