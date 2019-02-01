#include "material.h"

Material::Material() : Material(glm::vec3(0, 1, 0)) {}

Material::Material(float color_r, float color_g, float color_b) : Material(glm::vec3(color_r, color_g, color_b)) {}

Material::Material(const glm::vec3& color) : color_(color) {}
