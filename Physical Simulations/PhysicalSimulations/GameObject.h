#pragma once
#include <detail/type_mat.hpp>
#include "Material.h"
#include "Model.h"
#include "TextureManager.h"
#include "glm.hpp"

class GameObject {
   public:
    GameObject();
    explicit GameObject(Model* model);
    virtual ~GameObject();

    void SetPosition(const glm::vec3& position);
    void SetScale(float x, float y, float z);
    void EulerRotate(float yaw, float pitch, float roll);
    void SetColor(const glm::vec3& color);
    void SetTextureIndex(TEXTURE texture_index);

    void Update();

    Material material_;

   private:
    Model* model_;
    TEXTURE texture_index_;
    glm::mat4 transform_;
};
