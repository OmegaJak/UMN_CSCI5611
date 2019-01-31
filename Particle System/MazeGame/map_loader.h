#pragma once
#include <vector>
#include "game_object.h"
#include "map.h"
#include "material.h"
#include "model.h"

class MapLoader {
   public:
    MapLoader();
    ~MapLoader();

    Map* LoadMap(const std::string& filename);

   private:
    static Material GetMaterialForCharacter(char c);
    void LoadAssets();
    GameObject* GetGround(glm::vec3 base_position) const;

    static glm::vec3 GetPositionForCoordinate(int i, int j);
    static bool IsDoor(char c);
    static bool IsKey(char c);

    Model* wall_model_ = nullptr;
    Model* door_model_ = nullptr;
    Model* key_model_ = nullptr;
    Model* start_model_ = nullptr;
    Model* goal_model_ = nullptr;
};
