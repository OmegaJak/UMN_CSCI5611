#pragma once
#include <detail/type_vec4.hpp>
#include <string>
#include <vector>

class Model {
   public:
    explicit Model(const std::string& file);

    void LoadTxt(const std::string& file);
    void LoadObj(const std::string& file);

    int NumElements() const;
    int NumVerts() const;

    std::vector<glm::vec4> Vertices() const;

    float* model_;
    int vbo_vertex_start_index_;

   private:
    int num_verts_;
};
