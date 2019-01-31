#pragma once

#define GLM_FORCE_RADIANS
#include <unordered_set>
#include "gtc/matrix_transform.hpp"

class Transformable : public std::enable_shared_from_this<Transformable> {
   public:
    Transformable();
    Transformable(const glm::vec3& position);
    virtual ~Transformable();

    void Rotate(float radians, const glm::vec3& around);
    void Translate(float x, float y, float z);
    void Translate(const glm::vec3& translate_by);
    void Scale(const glm::vec3& scale);
    void Scale(float scale);
    void ApplyMatrix(const glm::mat4 matrix);
    void ResetAndSetTranslation(const glm::vec3& translation);

    void AddChild(std::shared_ptr<Transformable> child);
    void RemoveChild(std::shared_ptr<Transformable> child);
    void ClearChildren();
    void SetParent(std::shared_ptr<Transformable> parent);
    void ClearParent();
    bool HasChild(std::shared_ptr<Transformable> child) const;
    bool IsParent(std::shared_ptr<Transformable> parent) const;

    void RecalculateWorldTransform();

    void NotifyChildrenOfUpdate();
    static void NotifyChildOfUpdate(std::shared_ptr<Transformable> child);

    glm::mat4 LocalTransform() const;
    glm::mat4 WorldTransform() const;

    float X() const;
    float Y() const;
    float Z() const;
    glm::vec3 WorldPosition() const;
    glm::vec3 LocalPosition() const;

   private:
    void Reset();
    void ResetLocalTransform();

    glm::mat4 local_transform_;  // This Transformable's local transformation
    glm::mat4 world_transform_;  // This transform in world coordinates, with all parent transformations applied
    std::weak_ptr<Transformable> parent_;
    std::unordered_set<std::shared_ptr<Transformable>> children_;
};
