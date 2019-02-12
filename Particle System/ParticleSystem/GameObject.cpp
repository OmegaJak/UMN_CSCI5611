#define GLM_FORCE_RADIANS
#include <SDL_stdinc.h>
#include <gtc/type_ptr.hpp>
#include <gtx/euler_angles.hpp>
#include "Constants.h"
#include "GameObject.h"
#include "ShaderManager.h"
#include "glad.h"

GameObject::GameObject() : GameObject(nullptr) {}

GameObject::GameObject(Model* model) : model_(model), texture_index_(UNTEXTURED) {
    transform_ = glm::mat4();
    material_ = Material(glm::vec3(1, 0, 1));
}

GameObject::~GameObject() = default;

void GameObject::SetTextureIndex(TEXTURE texture_index) {
    texture_index_ = texture_index;
}

void GameObject::Update() {
    if (model_ == nullptr) {
        printf("GameObject must be given a valid model before calling Update()\n");
        exit(1);
    }

    glUniformMatrix4fv(ShaderManager::EnvironmentShader.Attributes.model, 1, GL_FALSE,
                       glm::value_ptr(transform_));                                  // pass model matrix to shader
    glUniform1i(ShaderManager::EnvironmentShader.Attributes.texID, texture_index_);  // Set which texture to use
    glUniform1f(ShaderManager::EnvironmentShader.Attributes.specFactor, material_.specFactor_);
    if (texture_index_ == UNTEXTURED) {
        glUniform3fv(ShaderManager::EnvironmentShader.Attributes.color, 1,
                     glm::value_ptr(material_.color_));  // Update the color, if necessary
    }

    glDrawArrays(GL_TRIANGLES, model_->vbo_vertex_start_index_, model_->NumVerts());
}

void GameObject::SetPosition(const glm::vec3& position) {
    transform_[3] = glm::vec4(position, transform_[3][3]);
}

void GameObject::SetScale(float x, float y, float z) {
    transform_[0][0] = x;
    transform_[1][1] = y;
    transform_[2][2] = z;
}

void GameObject::EulerRotate(float yawDeg, float pitchDeg, float rollDeg) {
    auto mat = glm::eulerAngleYXZ(glm::radians(yawDeg), glm::radians(pitchDeg), glm::radians(rollDeg));
    transform_ = mat * transform_;
}

void GameObject::SetColor(const glm::vec3& color) {
    material_.color_ = color;
}
