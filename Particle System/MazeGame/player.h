#pragma once
#include <vector>
#include "camera.h"
#include "game_object.h"
#include "key.h"

class Player : public GameObject {
   public:
    Player(Camera* camera, Map* map);

    void Update() override;

    void UseKey();
    void DropKey();
    void Jump();
    void Crouch();
    void UnCrouch();
    bool IsCrouching() const;

   private:
    void RegenerateBoundingBox();
    void InitializeKeyLocation(Key* key);

    Camera* camera_;
    std::vector<glm::vec4> box_;
    Key* held_key_;

    float vertical_velocity = 0.0f, forward_velocity = 0.0f, right_velocity = 0.0f;
    bool on_ground = true;
    bool stuck_in_object = false;
    bool crouching = false;
};
