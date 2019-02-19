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
    GameObject _gravityCenter;
    Model* _cubeModel;
    Model* _sphereModel;
    Model* _tubeModel;
};
