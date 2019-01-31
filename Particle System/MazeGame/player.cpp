#define GLM_FORCE_RADIANS
#define NOMINMAX

#include <SDL.h>
#include <algorithm>
#include "constants.h"
#include "map.h"
#include "player.h"

Player::Player(Camera* camera, Map* map) : GameObject() {
    camera_ = camera;
    if (camera == nullptr) {
        printf("Player was given null camera. Exiting...\n");
        exit(1);
    }

    map_ = map;
    if (map == nullptr) {
        printf("Player was given null map. Exiting...\n");
        exit(1);
    }

    float num = 0.15f;
    box_ = std::vector<glm::vec4>(8);
    box_[0] = glm::vec4(-num, -num, -PLAYER_HALF_HEIGHT, 1);
    box_[1] = glm::vec4(num, -num, -PLAYER_HALF_HEIGHT, 1);
    box_[2] = glm::vec4(num, num, -PLAYER_HALF_HEIGHT, 1);
    box_[3] = glm::vec4(-num, num, -PLAYER_HALF_HEIGHT, 1);
    box_[4] = glm::vec4(-num, -num, PLAYER_HALF_HEIGHT, 1);
    box_[5] = glm::vec4(num, -num, PLAYER_HALF_HEIGHT, 1);
    box_[6] = glm::vec4(num, num, PLAYER_HALF_HEIGHT, 1);
    box_[7] = glm::vec4(-num, num, PLAYER_HALF_HEIGHT, 1);

    glm::vec3 start_position = map_->SpawnPosition();
    start_position.z = START_CAMERA_Z;
    camera->SetPosition(start_position);

    glm::vec3 look_position = map_->GoalPosition();
    look_position.z = start_position.z;
    camera->SetLookAt(look_position);

    transform->SetParent(camera->transform);
    RegenerateBoundingBox();

    held_key_ = nullptr;
}

void Player::Update() {
    float base_player_height = START_CAMERA_Z;
    if (crouching) base_player_height -= CROUCH_DISTANCE;

    //// Jump logic ////
    if (!on_ground) {
        vertical_velocity -= GRAVITY;
        if (transform->Z() + vertical_velocity < base_player_height) {
            vertical_velocity = base_player_height - transform->Z();
            on_ground = true;
        }
    } else {
        forward_velocity = 0;
        right_velocity = 0;
    }

    float move_speed = CAMERA_MOVE_SPEED;
    if (!on_ground && !stuck_in_object) {
        move_speed = move_speed * JUMPING_LATERAL_MOVEMENT_FACTOR;
    }
    if (crouching) {
        move_speed *= CROUCH_SPEED_FACTOR;
    }

    //// Player movement ////
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_RIGHT]) {
        camera_->Rotate(0, -CAMERA_ROTATION_SPEED);
    } else if (key_state[SDL_SCANCODE_LEFT]) {
        camera_->Rotate(0, CAMERA_ROTATION_SPEED);
    }

    if (key_state[SDL_SCANCODE_W]) {
        forward_velocity += move_speed;  // movement forward
    } else if (key_state[SDL_SCANCODE_S]) {
        forward_velocity -= move_speed;
    }
    if (key_state[SDL_SCANCODE_D]) {
        right_velocity += move_speed;
    } else if (key_state[SDL_SCANCODE_A]) {
        right_velocity -= move_speed;
    }

    // Clamp the forward and right velocities
    forward_velocity = std::max(std::min(forward_velocity, MAX_MOVE_SPEED), -MAX_MOVE_SPEED);
    right_velocity = std::max(std::min(right_velocity, MAX_MOVE_SPEED), -MAX_MOVE_SPEED);

    camera_->Translate(right_velocity, vertical_velocity, forward_velocity);

    RegenerateBoundingBox();

    if (map_->IntersectsAnySolidObjects(this)) {
        camera_->Translate(-right_velocity, -vertical_velocity, -forward_velocity);  // Undo the movement
        RegenerateBoundingBox();

        right_velocity = 0;
        forward_velocity = 0;
        stuck_in_object = true;
    } else {
        stuck_in_object = false;
    }

    // printf("Player bounds: min: %f, %f, %f, max:: %f, %f, %f\n", bounding_box_->Min().x, bounding_box_->Min().y, bounding_box_->Min().z,
    //       bounding_box_->Max().x, bounding_box_->Max().y, bounding_box_->Max().z);
    // printf("Player z: %f\n", transform->Z());

    //// Key logic ////
    Key* key = map_->FirstIntersectedKey(this);
    if (key != nullptr && held_key_ == nullptr && key->CanBePickedUp()) {
        held_key_ = key;
        held_key_->SetHolder(this);
        InitializeKeyLocation(held_key_);
    }

    //// Reset movement variables ////
    if (on_ground) {
        vertical_velocity = 0;
    }
    on_ground = transform->Z() <= base_player_height;
}

void Player::UseKey() {
    held_key_ = nullptr;
}

void Player::DropKey() {
    if (held_key_ == nullptr) return;

    held_key_->Drop();
    held_key_ = nullptr;
}

void Player::Jump() {
    if (on_ground) {
        vertical_velocity = JUMP_VELOCITY;
        on_ground = false;
    }
}

void Player::Crouch() {
    if (!crouching) {
        crouching = true;
        camera_->transform->Translate(0, 0, -CROUCH_DISTANCE);
    }
}

void Player::UnCrouch() {
    crouching = false;
    camera_->transform->Translate(0, 0, CROUCH_DISTANCE);
}

bool Player::IsCrouching() const {
    return crouching;
}

void Player::InitializeKeyLocation(Key* key) {
    held_key_->transform->ResetAndSetTranslation(camera_->GetNormalizedLookPosition() * 0.2f);
    held_key_->transform->Translate(0, 0, -0.1);
    held_key_->transform->Scale(0.3f);
    held_key_->transform->Rotate(M_PI / 2, glm::vec3(1, 0, 0));
    held_key_->transform->Rotate(3 * M_PI / 4, glm::vec3(0, 0, 1));

    held_key_->transform->SetParent(transform);
}

void Player::RegenerateBoundingBox() {
    InitBoundingBox(box_);
    bounding_box_->transform->ClearParent();
}
