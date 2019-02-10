#include "Environment.h"

Environment::Environment() {
    _cubeModel = new Model("models/cube.txt");
    _sphereModel = new Model("models/sphere.txt");

    CreateEnvironment();
}

void Environment::UpdateAll() {
    for (auto gameObject : _gameObjects) {
        gameObject.Update();
    }
}

void Environment::SetGravityCenterPosition(const glm::vec3& position) {
    _gameObjects[_gameObjects.size() - 1].SetPosition(position);
}

void Environment::CreateEnvironment() {
    auto z = -0.5;

    auto num = 10;
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < num; j++) {
            auto gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(i, j, z));
            _gameObjects.push_back(gameObject);

            gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(-1, i, j));
            _gameObjects.push_back(gameObject);

            gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(num, i, j));
            _gameObjects.push_back(gameObject);

            gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(i, -1, j));
            _gameObjects.push_back(gameObject);

            gameObject = GameObject(_cubeModel);
            gameObject.SetTextureIndex(TEX0);
            gameObject.SetPosition(glm::vec3(i, num, j));
            _gameObjects.push_back(gameObject);
        }
    }

    auto gameObject = GameObject(_sphereModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0, 0, 0));
    gameObject.SetScale(5, 5, 5);
    _gravityCenter = gameObject;
    _gameObjects.push_back(gameObject);
}
