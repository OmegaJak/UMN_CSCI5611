#include "ClothManager.h"
#include "Environment.h"
#include "Utils.h"

Environment::Environment() {
    _cubeModel = new Model("models/cube.txt");
    _sphereModel = new Model("models/sphere.txt");

    CreateEnvironment();
}

void Environment::UpdateAll() {
    for (auto gameObject : _gameObjects) {
        gameObject.Update();
    }

    for (auto gameObject : masses) {
        gameObject.Update();
    }

    for (auto gameObject : springs) {
        gameObject.Update();
    }
}

void Environment::SetGravityCenterPosition(const glm::vec3& position) {
    _gameObjects[_gravityCenterIndex].SetPosition(position);
}

void Environment::CreateEnvironment() {
    GameObject gameObject;

    gameObject = GameObject(_cubeModel);  // ground
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0, 77 / 255.0, 26 / 255.0));
    gameObject.SetScale(20, 20, 1);
    gameObject.SetPosition(glm::vec3(0, 0, -0.55));
    gameObject.material_.specFactor_ = 0.2;
    _gameObjects.push_back(gameObject);

    gameObject = GameObject(_cubeModel);  // reference person
    gameObject.SetTextureIndex(TEX1);
    gameObject.SetScale(1, 0, -3);
    gameObject.SetPosition(glm::vec3(-25, 13, 1.5));
    _gameObjects.push_back(gameObject);

    gameObject = GameObject(_sphereModel);
    gameObject.SetTextureIndex(UNTEXTURED);
    gameObject.SetColor(glm::vec3(0, 0, 0));
    gameObject.SetPosition(glm::vec3(10, 10, 0));
    _gameObjects.push_back(gameObject);
    _gravityCenterIndex = _gameObjects.size() - 1;

    for (auto& mass : masses) {
        gameObject = GameObject(_sphereModel);
        gameObject.SetTextureIndex(UNTEXTURED);
        gameObject.SetColor(glm::vec3((101 + Utils::randBetween(0, 50)) / 255.0, 67 / 255.0, 33 / 255.0));
        gameObject.SetScale(0.5, 0.5, 0.5);
        mass = gameObject;
    }

    for (auto& mass : springs) {
        gameObject = GameObject(_cubeModel);
        gameObject.SetTextureIndex(UNTEXTURED);
        gameObject.SetColor(glm::vec3(1, 0, 0));
        gameObject.SetScale(0.25, 0.25, 0.25);
        mass = gameObject;
    }
}
