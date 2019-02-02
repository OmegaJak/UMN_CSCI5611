#pragma once
#include "material.h"
#include "model.h"
#include "TextureManager.h"

class GameObject {
   public:
    GameObject();
    explicit GameObject(Model* model);
    virtual ~GameObject();

    void SetTextureIndex(TEXTURE texture_index);

    void Update();

    Material material_;

   private:
    Model* model_;
    TEXTURE texture_index_;
    glm::mat4 transform_;
};
