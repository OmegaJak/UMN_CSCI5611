#include "goal.h"
#include "map.h"

void Goal::Update() {
    if (map_->IntersectsPlayer(this)) {
        printf("Congratulations! You successfully completed the maze!\n");
        exit(0);
    }

    GameObject::Update();
}
