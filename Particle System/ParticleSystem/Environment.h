#pragma once
#include "GameObject.h"

class Environment {
   public:
    Environment();

    void UpdateAll();

   private:
    void CreateEnvironment();

    std::vector<GameObject> _gameObjects;
    Model* _cubeModel;
};
