#include "material.h"

Material::Material() : Material(glm::vec3(0, 1, 0)) {}

Material::Material(float color_r, float color_g, float color_b, float specFactor)
    : Material(glm::vec3(color_r, color_g, color_b), specFactor) {}

Material::Material(const glm::vec3& color, float specFactor) : color_(color), specFactor_(specFactor) {}
