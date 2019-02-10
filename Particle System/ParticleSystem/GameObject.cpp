#define GLM_FORCE_RADIANS
#include <gtc/type_ptr.hpp>
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

    /*glVertexAttribPointer(ShaderManager::EnvironmentAttributes.position, VALUES_PER_POSITION, GL_FLOAT, GL_FALSE,
                          ATTRIBUTE_STRIDE * sizeof(float), POSITION_OFFSET);*/

    glUniformMatrix4fv(ShaderManager::EnvironmentAttributes.model, 1, GL_FALSE, glm::value_ptr(transform_));  // pass model matrix to shader
    glUniform1i(ShaderManager::EnvironmentAttributes.texID, texture_index_);                                  // Set which texture to use
    if (texture_index_ == UNTEXTURED) {
        glUniform3fv(ShaderManager::EnvironmentAttributes.color, 1, glm::value_ptr(material_.color_));  // Update the color, if necessary
    }

    glDrawArrays(GL_TRIANGLES, model_->vbo_vertex_start_index_, model_->NumVerts());
}

void GameObject::SetPosition(const glm::vec3& position) {
    transform_[3] = glm::vec4(position, transform_[3][3]);
}
