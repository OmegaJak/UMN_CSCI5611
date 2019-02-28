#pragma once
#include <vector>
#include "Model.h"
#include "glad.h"

class ModelManager {
   public:
    static void RegisterModel(Model* model);

    static void InitVBO();
    static void Cleanup();

    static int NumElements();

   private:
    static std::vector<Model*> models_;
    static int num_verts_;
};
