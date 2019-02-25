#pragma once
#include "GameObject.h"

class Environment {
   public:
    Environment();

    void UpdateAll();
    void SetGravityCenterPosition(const glm::vec3& position);

    GameObject masses[ClothManager::NUM_MASSES];
    GameObject springs[ClothManager::NUM_SPRINGS];

   private:
    void CreateEnvironment();

    std::vector<GameObject> _gameObjects;
    int _gravityCenterIndex;
    Model* _cubeModel;
    Model* _sphereModel;
};
