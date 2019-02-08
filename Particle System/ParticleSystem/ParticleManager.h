#pragma once
#include <detail/type_vec3.hpp>
#include <vector>
#include "Model.h"

class ParticleManager {
   public:
    ParticleManager();

    void MoveParticles(float dt);
    void SpawnParticle(const glm::vec3& position, const glm::vec3& velocity);
    void SpawnParticles(float dt);
    void RenderParticles();
    int GetNumParticles() const;

    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Velocities;
    std::vector<float> Lifetimes;

    float genRate = 1000;

   private:
    void DeleteParticle(int index);
    static float rand01();

    Model* _particleModel;
};
