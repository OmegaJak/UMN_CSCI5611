#pragma once
#include <detail/type_vec3.hpp>

class Material {
   public:
    Material();
    Material(float color_r, float color_g, float color_b, float specFactor = 0.7);
    explicit Material(const glm::vec3& color, float specFactor = 0.7);

    glm::vec3 color_;
    float specFactor_;
};
