#include <vector>
#include "map.h"
#include "player.h"

Map::Map() {
    player_ = nullptr;
    goal_ = nullptr;
    spawn_ = nullptr;
}

Map::~Map() = default;

void Map::Add(GameObject* object) {
    if (dynamic_cast<Wall*>(object)) {
        walls_.push_back(dynamic_cast<Wall*>(object));
    } else if (dynamic_cast<Player*>(object)) {
        player_ = dynamic_cast<Player*>(object);
    } else if (dynamic_cast<Door*>(object)) {
        doors_.push_back(dynamic_cast<Door*>(object));
    } else if (dynamic_cast<Key*>(object)) {
        keys_.push_back(dynamic_cast<Key*>(object));
    } else if (dynamic_cast<Spawn*>(object)) {
        spawn_ = dynamic_cast<Spawn*>(object);
    } else if (dynamic_cast<Goal*>(object)) {
        goal_ = dynamic_cast<Goal*>(object);
    }

    all_elements_.push_back(object);
}

void Map::Init() {}

void Map::UpdateAll() {
    for (auto game_object : all_elements_) {
        game_object->Update();
    }
}

bool Map::IntersectsAnySolidObjects(GameObject* object) {
    for (auto element : all_elements_) {
        if (element->IsSolid() && object->IntersectsWith(*element)) return true;
    }

    return false;
}

Player* Map::IntersectsPlayer(GameObject* object) {
    if (object->IntersectsWith(*player_)) {
        return player_;
    }

    return nullptr;
}

Key* Map::FirstIntersectedKey(GameObject* object) {
    for (auto key : keys_) {
        if (key->IntersectsWith(*object)) return key;
    }

    return nullptr;
}

Door* Map::IntersectsDoorWithId(GameObject* object, char id) {
    for (auto door : doors_) {
        if (door->MatchesId(id) && door->IntersectsWith(*object)) return door;
    }

    return nullptr;
}

glm::vec3 Map::SpawnPosition() const {
    if (spawn_ == nullptr) {
        printf("Can't get spawn position when we don't have a spawn...\n");
        exit(1);
    }
    return spawn_->transform->WorldPosition();
}

glm::vec3 Map::GoalPosition() const {
    if (goal_ == nullptr) {
        printf("Can't get goal position when we don't have a goal...\n");
        exit(1);
    }
    return goal_->transform->WorldPosition();
}
