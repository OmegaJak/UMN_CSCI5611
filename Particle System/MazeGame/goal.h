#pragma once
#include "game_object.h"

class Map;

class Goal : public GameObject {
   public:
    Goal(Model* model, Map* map) : GameObject(model, map) {}
    ~Goal() = default;

    void Update() override;
};
