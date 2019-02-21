// Much of this was based on the camera found here https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h

#include <SDL.h>
#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
#include "Camera.h"
#include "Constants.h"
#include "ShaderManager.h"
#include "glad.h"
#include "gtx/rotate_vector.hpp"

Camera::Camera() {
    _position = glm::vec3(-5, 0, 5);
    _forward = glm::vec3(1, 0, 0);
    _worldUp = _up = glm::vec3(0, 0, 1);

    _yaw = 0.0f;
    _pitch = 0.0f;

    UpdateCameraVectors();
}

void Camera::ProcessMouseInput(float deltaX, float deltaY, bool constrainPitch) {
    const float mouseSensitivity = -0.1f;
    deltaX *= mouseSensitivity;
    deltaY *= mouseSensitivity;

    _yaw += deltaX;
    _pitch += deltaY;

    if (constrainPitch) {
        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < -89.0f) _pitch = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessKeyboardInput() {
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    auto rotateSpeed = CAMERA_ROTATION_SPEED;
    auto moveSpeed = CAMERA_MOVE_SPEED;
    if (key_state[SDL_SCANCODE_LSHIFT]) {
        moveSpeed = MAX_MOVE_SPEED;
    }

    // Look up/down
    if (key_state[SDL_SCANCODE_UP]) {
        _pitch += rotateSpeed;
    } else if (key_state[SDL_SCANCODE_DOWN]) {
        _pitch -= rotateSpeed;
    }

    // Look right/left
    if (key_state[SDL_SCANCODE_RIGHT]) {
        _yaw -= rotateSpeed;
    } else if (key_state[SDL_SCANCODE_LEFT]) {
        _yaw += rotateSpeed;
    }

    // Forward/back
    if (key_state[SDL_SCANCODE_W]) {
        _position += _forward * moveSpeed;
    } else if (key_state[SDL_SCANCODE_S]) {
        _position -= _forward * moveSpeed;
    }

    // Right/left
    if (key_state[SDL_SCANCODE_D]) {
        _position += _right * moveSpeed;
    } else if (key_state[SDL_SCANCODE_A]) {
        _position -= _right * moveSpeed;
    }

    // Up/down
    if (key_state[SDL_SCANCODE_R]) {
        _position += _up * moveSpeed;
    } else if (key_state[SDL_SCANCODE_F]) {
        _position -= _up * moveSpeed;
    }

    UpdateCameraVectors();
}

glm::vec3 Camera::GetPosition() {
    return _position;
}

glm::vec3 Camera::GetMousePosition(float normalizedMouseX, float normalizedMouseY, const glm::mat4& proj, float distanceFromCamera) {
    glm::mat4 invVP = glm::inverse(proj * _view);
    glm::vec4 screenPos = glm::vec4(normalizedMouseX, -normalizedMouseY, 1.0f, 1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 direction = glm::normalize(glm::vec3(worldPos));
    glm::vec3 mousePosition = _position + distanceFromCamera * direction;

    return mousePosition;
}

glm::vec3 Camera::GetForward() {
    return _forward;
}

void Camera::Update() {
    ProcessKeyboardInput();
    auto view = glm::lookAt(_position, _position + _forward, _up);
    _view = view;

    ShaderManager::ApplyToEachRenderShader(
        [view](ShaderAttributes attributes) -> void { glUniformMatrix4fv(attributes.view, 1, GL_FALSE, glm::value_ptr(view)); },
        VIEW_SHADER_FUNCTION_ID);
}

void Camera::UpdateCameraVectors() {
    glm::vec3 forward;
    forward.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    forward.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    forward.z = sin(glm::radians(_pitch));
    _forward = glm::normalize(forward);

    _right = glm::normalize(glm::cross(_forward, _worldUp));
    _up = glm::normalize(glm::cross(_right, _forward));
}
