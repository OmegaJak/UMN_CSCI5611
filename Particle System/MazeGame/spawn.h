#pragma once
#include "game_object.h"

class Spawn : public GameObject {
   public:
    Spawn(Model* model) : GameObject(model) {}
    ~Spawn() = default;
};
