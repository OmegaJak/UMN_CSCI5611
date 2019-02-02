
#include <SDL.h>
#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
#include "Camera.h"
#include "Constants.h"
#include "glad.h"
#include "gtx/rotate_vector.hpp"
#include "ShaderManager.h"

Camera::Camera() {
    position_ = glm::vec3(3, 0, 0);
    look_at_ = glm::vec3(0, 0, 0);
    up_ = glm::vec3(0, 0, 1);
}

Camera::~Camera() {}

void Camera::Rotate(float vertical_rotation, float horizontal_rotation, float roll_rotation) {
    if (abs(vertical_rotation) > ABSOLUTE_TOLERANCE) {  // Avoid the computations if we can
        glm::vec3 right = glm::cross(look_at_ - position_, up_);

        look_at_ = glm::rotate(look_at_ - position_, vertical_rotation, right) + position_;
        up_ = glm::rotate(up_, vertical_rotation, right);
    }

    if (abs(horizontal_rotation) > ABSOLUTE_TOLERANCE) {
        look_at_ = glm::rotate(look_at_ - position_, -horizontal_rotation, up_) + position_;
    }

    if (abs(roll_rotation) > ABSOLUTE_TOLERANCE) {
        up_ = glm::rotate(up_, roll_rotation, look_at_ - position_);
    }
}

void Camera::Translate(float right, float up, float forward) {
    glm::vec3 forward_vec = glm::normalize(look_at_ - position_);
    glm::vec3 right_vec = glm::normalize(glm::cross(forward_vec, up_));

    glm::vec3 translation = right * right_vec + up * up_ + forward * forward_vec;
    position_ += translation;
    look_at_ += translation;
}

void Camera::SetPosition(const glm::vec3& position) {
    position_ = position;
}

void Camera::SetLookAt(const glm::vec3& look_at_position) {
    look_at_ = look_at_position;
}

void Camera::SetUp(const glm::vec3& up) {
    up_ = up;
}

void Camera::Update() {
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_UP]) {
        Rotate(CAMERA_ROTATION_SPEED, 0);
    } else if (key_state[SDL_SCANCODE_DOWN]) {
        Rotate(-CAMERA_ROTATION_SPEED, 0);
    }
    if (key_state[SDL_SCANCODE_RIGHT]) {
        Rotate(0, CAMERA_ROTATION_SPEED);
    } else if (key_state[SDL_SCANCODE_LEFT]) {
        Rotate(0, -CAMERA_ROTATION_SPEED);
    }
    if (key_state[SDL_SCANCODE_E]) {
        Rotate(0, 0, CAMERA_ROTATION_SPEED);
    } else if (key_state[SDL_SCANCODE_Q]) {
        Rotate(0, 0, -CAMERA_ROTATION_SPEED);
    }

    if (key_state[SDL_SCANCODE_W]) {
        Translate(0, 0, CAMERA_MOVE_SPEED);
    } else if (key_state[SDL_SCANCODE_S]) {
        Translate(0, 0, -CAMERA_MOVE_SPEED);
    }
    if (key_state[SDL_SCANCODE_D]) {
        Translate(CAMERA_MOVE_SPEED, 0, 0);
    } else if (key_state[SDL_SCANCODE_A]) {
        Translate(-CAMERA_MOVE_SPEED, 0, 0);
    }
    if (key_state[SDL_SCANCODE_R]) {
        Translate(0, CAMERA_MOVE_SPEED, 0);
    } else if (key_state[SDL_SCANCODE_F]) {
        Translate(0, -CAMERA_MOVE_SPEED, 0);
    }

    glm::mat4 view = glm::lookAt(position_, look_at_, up_);
    glUniformMatrix4fv(ShaderManager::Attributes.view, 1, GL_FALSE, glm::value_ptr(view));
}
