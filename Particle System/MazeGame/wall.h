#pragma once
#include "game_object.h"

class Wall : public GameObject {
   public:
    Wall(Model* model) : GameObject(model) {}
    Wall(Model* model, bool solid) : GameObject(model) {
        is_solid_ = solid;
    }
    ~Wall() = default;

    bool IsSolid() override {
        return is_solid_;
    }

   private:
    bool is_solid_ = true;
};
