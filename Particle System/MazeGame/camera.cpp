#define GLM_FORCE_RADIANS

#include <SDL.h>
#include <gtc/type_ptr.hpp>
#include "camera.h"
#include "constants.h"
#include "glad.h"
#include "gtx/rotate_vector.hpp"
#include "shader_manager.h"

Camera::Camera() {
    transform = std::make_shared<Transformable>(glm::vec3(0, 0, 0.5));
    look_at_ = std::make_shared<Transformable>(glm::vec3(3, 0, 0.5));
    up_ = std::make_shared<Transformable>(glm::vec3(0, 0, 1));

    look_at_->SetParent(transform);
    up_->SetParent(transform);
}

Camera::~Camera() {}

void Camera::Rotate(float vertical_rotation, float horizontal_rotation, float roll_rotation) {
    if (abs(vertical_rotation) > ABSOLUTE_TOLERANCE) {  // Avoid the computations if we can
        glm::vec3 right = glm::cross(look_at_->LocalPosition(), up_->LocalPosition());

        transform->Rotate(vertical_rotation, right);
    }

    if (abs(horizontal_rotation) > ABSOLUTE_TOLERANCE) {
        transform->Rotate(horizontal_rotation, up_->LocalPosition());
    }

    if (abs(roll_rotation) > ABSOLUTE_TOLERANCE) {
        transform->Rotate(roll_rotation, look_at_->LocalPosition());
    }
}

void Camera::Translate(float right, float up, float forward) {
    glm::vec3 forward_vec = glm::normalize(look_at_->LocalPosition());
    glm::vec3 right_vec = glm::normalize(glm::cross(forward_vec, up_->LocalPosition()));

    glm::vec3 translation = right * right_vec + up * up_->LocalPosition() + forward * forward_vec;
    transform->Translate(translation);
}

void Camera::SetPosition(const glm::vec3& position) {
    transform->ResetAndSetTranslation(position);
}

void Camera::SetLookAt(const glm::vec3& look_at_position) {
    look_at_->ResetAndSetTranslation(look_at_position - transform->WorldPosition());
}

glm::vec3 Camera::GetNormalizedLookPosition() const {
    return glm::normalize(look_at_->LocalPosition());
}

float Camera::GetTotalHorizontalRotation() const {
    return total_horizontal_rotation;
}

void Camera::Update() {
    /*const Uint8* key_state = SDL_GetKeyboardState(NULL);
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
    }*/

    glm::mat4 view = glm::lookAt(transform->WorldPosition(), look_at_->WorldPosition(), up_->WorldPosition() - transform->WorldPosition());
    // printf("CameraPos: %f, %f, %f\n", position_.x, position_.y, position_.z);
    glUniformMatrix4fv(ShaderManager::Attributes.view, 1, GL_FALSE, glm::value_ptr(view));
}
