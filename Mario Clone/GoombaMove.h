#pragma once
#include "MoveComponent.h"
#include "Math.h"
#include <vector>

class GoombaMove : public MoveComponent
{
public:
    GoombaMove(class Actor *owner);
    void Update(float deltaTime) override;
    void ProcessInput(const Uint8 *keyState) override;
    const float speed = 100.0f;
    float mYspeed = 0;
    const float G = 2000.0f;
    const float bottom = 448.0f;
    bool mInAir = false;

private:
    template <typename t>
    bool checkCollisionsWithCollection(std::vector<t>& collection);
};