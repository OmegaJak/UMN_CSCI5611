#include "Environment.h"

Environment::Environment() {
    _cubeModel = new Model("models/cube.txt");

    CreateEnvironment();
}

void Environment::UpdateAll() {
    for (auto gameObject : _gameObjects) {
        gameObject.Update();
    }
}

void Environment::CreateEnvironment() {
    auto z = -0.5;

    auto num = 100;
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < num; j++) {
            auto gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(i, j, z));
            _gameObjects.push_back(gameObject);
        }
    }
}
