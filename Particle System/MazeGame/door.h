#pragma once
#include "game_object.h"

class Door : public GameObject {
   public:
    Door(Model* model, char id);
    ~Door() = default;

    bool IsSolid() override {
        return true;
    }

    bool MatchesId(char id);
    void GoAway();

    void Update() override;

   private:
    char id_;
    bool is_going_away = false;
    float scale = 1.0f;
};
