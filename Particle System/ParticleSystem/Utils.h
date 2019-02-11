#pragma once
#include <cstdlib>

class Utils {
   public:
    // https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
    static float randBetween(int min, int max) {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = max - min;
        float r = random * diff;
        return min + r;
    }
};
