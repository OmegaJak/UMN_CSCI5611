#pragma once
#include <memory>
#include <vector>
#include "transformable.h"

/**
 * Known problem: this does nothing to account for rotation or shearing of the parent transform.
 */
class BoundingBox {
   public:
    BoundingBox();
    explicit BoundingBox(std::vector<glm::vec4> points);  // Grows BB to encompass all points
    ~BoundingBox();

    void ExpandToBound(const BoundingBox& other);
    void ExpandToBound(const std::vector<BoundingBox> bounding_boxes);

    bool ContainsOrIntersects(const BoundingBox& other) const;

    glm::vec3 Max() const;
    glm::vec3 Min() const;

    std::shared_ptr<Transformable> transform;

   private:
    static bool Overlaps(double otherMin, double otherMax, double min, double max);
    static bool IsBetween(double val, double low, double high);
    void InitToBounds(glm::vec3 min, glm::vec3 max);

    std::shared_ptr<Transformable> min_;
    std::shared_ptr<Transformable> max_;
};
