#pragma once
#include "GameObject.h"

class Environment {
   public:
    Environment();

    void UpdateAll();
    void SetGravityCenterPosition(const glm::vec3& position);

   private:
    void CreateEnvironment();

    std::vector<GameObject> _gameObjects;
    int _gravityCenterIndex;
    Model* _cubeModel;
    Model* _sphereModel;
};
