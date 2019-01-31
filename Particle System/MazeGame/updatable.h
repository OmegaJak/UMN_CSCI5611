#pragma once
class Updatable {
   public:
    Updatable() = default;
    virtual ~Updatable() = default;

    virtual void Update() = 0;
};
