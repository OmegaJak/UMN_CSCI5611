#pragma once
#include <detail/type_vec3.hpp>
#include <vector>
#include "Model.h"

class ParticleManager {
   public:
    ParticleManager();

    void MoveParticles(float dt);
    void SpawnParticle(const glm::vec3& position, const glm::vec3& velocity);
    void RenderParticles();

    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Velocities;
    std::vector<float> Lifetimes;

   private:
    void DeleteParticle(int index);

    Model* _particleModel;
};
