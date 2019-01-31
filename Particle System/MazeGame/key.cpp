#include <SDL.h>
#include <cmath>
#include "constants.h"
#include "key.h"
#include "map.h"
#include "player.h"

Key::Key(Model* model, Map* map, char id, glm::vec2 pos) : GameObject(model, map) {
    id_ = id;

    transform->Translate(glm::vec3(pos.x, pos.y, 0));
    InitTransform();
}

void Key::Update() {
    Door* door = map_->IntersectsDoorWithId(this, id_);
    if (holder_ != nullptr && door != nullptr) {
        door->GoAway();
        GoAway();
        holder_->UseKey();
        holder_ = nullptr;
    }

    if (holder_ == nullptr) {
        transform->Rotate(KEY_ROTATION_SPEED, glm::vec3(0, 1, 0));

        int time = SDL_GetTicks();
        transform->Translate(0, cos(time / 700.0f) / 1000, 0);
    }

    GameObject::Update();
}

void Key::GoAway() {
    transform->ClearParent();
    transform->ResetAndSetTranslation(glm::vec3(0, 0, -3));
}

void Key::SetHolder(Player* player) {
    holder_ = player;
}

void Key::Drop() {
    holder_ = nullptr;
    InitTransform();
    drop_time_ = SDL_GetTicks();
}

bool Key::CanBePickedUp() {
    return SDL_GetTicks() - drop_time_ > KEY_DROP_PICKUP_COOLDOWN_MS;
}

void Key::InitTransform() {
    glm::vec2 previous_pos = glm::vec2(transform->X(), transform->Y());
    transform->ClearParent();
    transform->ResetAndSetTranslation(glm::vec3(previous_pos, KEY_HEIGHT));
    transform->Rotate(M_PI / 2, glm::vec3(1, 0, 0));
}
