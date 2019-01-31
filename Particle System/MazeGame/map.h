#pragma once
#include <detail/type_vec3.hpp>
#include <vector>
#include "door.h"
#include "game_object.h"
#include "goal.h"
#include "key.h"
#include "player.h"
#include "spawn.h"
#include "wall.h"

class Map {
   public:
    Map();
    ~Map();

    void Add(GameObject* object);
    void Init();

    void UpdateAll();
    bool IntersectsAnySolidObjects(GameObject* object);
    Player* IntersectsPlayer(GameObject* object);
    Key* FirstIntersectedKey(GameObject* object);
    Door* IntersectsDoorWithId(GameObject* object, char id);

    glm::vec3 SpawnPosition() const;
    glm::vec3 GoalPosition() const;

   private:
    std::vector<GameObject*> all_elements_;
    std::vector<Wall*> walls_;
    std::vector<Door*> doors_;
    std::vector<Key*> keys_;
    Spawn* spawn_;
    Goal* goal_;
    Player* player_;
};
