#include <algorithm>
#include "Constants.h"
#include "ModelManager.h"
#include "ShaderManager.h"

void ModelManager::RegisterModel(Model* model) {
    models_.push_back(model);
    model->vbo_vertex_start_index_ = num_verts_;
    num_verts_ += model->NumVerts();
}

void ModelManager::InitVBO() {
    float* model_data = new float[NumElements()];

    int current_offset = 0;
    for (auto model : models_) {
        std::copy(model->model_, model->model_ + model->NumElements(), model_data + current_offset);
        current_offset += model->NumElements();
    }

    glGenBuffers(1, &ShaderManager::EnvironmentShader.VBO);  // Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER,
                 ShaderManager::EnvironmentShader.VBO);  // Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, NumElements() * sizeof(float), model_data, GL_STATIC_DRAW);  // upload vertices to vbo
}

void ModelManager::Cleanup() {
    glDeleteBuffers(1, &ShaderManager::EnvironmentShader.VBO);
}

int ModelManager::NumElements() {
    return num_verts_ * ELEMENTS_PER_VERT;
}

std::vector<Model*> ModelManager::models_;
int ModelManager::num_verts_;
