#pragma once
#include <gtc/matrix_transform.hpp>

class Camera {
   public:
    Camera();

    void ProcessMouseInput(float deltaX, float deltaY, bool constrainPitch = true);
    void ProcessKeyboardInput();

    void Update();

   private:
    void UpdateCameraVectors();

    glm::vec3 _position, _forward, _up, _right, _worldUp;
    float _yaw, _pitch;
};
