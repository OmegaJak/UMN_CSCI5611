#pragma once
#include "bounding_box.h"
#include "material.h"
#include "model.h"
#include "texture_manager.h"
#include "transformable.h"
#include "updatable.h"

class Map;

class GameObject : public Updatable {
   public:
    GameObject();
    GameObject(Model* model);
    GameObject(Model* model, Map* map);
    virtual ~GameObject();

    void SetTextureIndex(TEXTURE texture_index);

    void Update() override;
    bool IntersectsWith(const GameObject& other) const;
    virtual bool IsSolid() {  // To be overriden by child classes
        return false;
    }

    Material material;
    std::shared_ptr<Transformable> transform;

   protected:
    void InitBoundingBox(const std::vector<glm::vec4>& vertices);
    glm::vec4 ToWorldSpace(const glm::vec4& model_coordinate) const;

    std::shared_ptr<BoundingBox> bounding_box_;
    Model* model_;
    TEXTURE texture_index_;
    Map* map_;
};
