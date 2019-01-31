#pragma once
#include <detail/type_vec3.hpp>

class Material {
   public:
    Material();
    Material(float color_r, float color_g, float color_b);
    explicit Material(const glm::vec3& color);

    glm::vec3 color_;
};
