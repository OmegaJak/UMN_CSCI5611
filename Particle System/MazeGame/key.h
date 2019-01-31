#pragma once
#include "game_object.h"

class Map;
class Player;

class Key : public GameObject {
   public:
    explicit Key(Model* model, Map* map, char id, glm::vec2 pos);
    ~Key() = default;

    void Update() override;
    void GoAway();
    void SetHolder(Player* player);
    void Drop();
    bool CanBePickedUp();

   private:
    void InitTransform();

    char id_;
    Player* holder_;
    int drop_time_ = 0;
};
