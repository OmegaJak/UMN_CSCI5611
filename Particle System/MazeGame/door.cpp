#include "constants.h"
#include "door.h"

Door::Door(Model* model, char id) : GameObject(model) {
    id_ = id;
}

bool Door::MatchesId(char id) {
    return tolower(id) == tolower(id_);
}

void Door::GoAway() {
    is_going_away = true;
}

void Door::Update() {
    if (is_going_away) {
        scale *= DOOR_SHRINK_FACTOR;
        transform->Scale(DOOR_SHRINK_FACTOR);
        transform->Rotate(DOOR_ROTATION_SPEED, glm::vec3(0, 1, 1));
    }

    if (scale < MIN_DOOR_SCALE) {
        is_going_away = false;
        transform->Translate(0, 0, -1000);  // This isn't the 'right' way to do this but it works
    }

    GameObject::Update();
}
