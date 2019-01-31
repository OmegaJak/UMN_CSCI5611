#pragma once
#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>
#include <memory>
#include "transformable.h"
#include "updatable.h"

class Camera : public Updatable {
   public:
    Camera();
    ~Camera() override;

    void Rotate(float vertical_rotation, float horizontal_rotation, float roll_rotation = 0);
    void Translate(float right, float up, float forward);

    void SetPosition(const glm::vec3& position);
    void SetLookAt(const glm::vec3& look_at_position);

    glm::vec3 GetNormalizedLookPosition() const;
    float GetTotalHorizontalRotation() const;

    void Update() override;

    std::shared_ptr<Transformable> transform;

   private:
    float total_horizontal_rotation = 0;

    std::shared_ptr<Transformable> look_at_, up_;
};
