#include <algorithm>
#include "constants.h"
#include "model_manager.h"

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

    glGenBuffers(1, &vbo_);               // Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);  // Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, NumElements() * sizeof(float), model_data, GL_STATIC_DRAW);  // upload vertices to vbo
}

void ModelManager::Cleanup() {
    glDeleteBuffers(1, &vbo_);
}

int ModelManager::NumElements() {
    return num_verts_ * ELEMENTS_PER_VERT;
}

std::vector<Model*> ModelManager::models_;
int ModelManager::num_verts_;
GLuint ModelManager::vbo_;
