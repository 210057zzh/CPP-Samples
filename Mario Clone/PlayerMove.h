#pragma once
#include "MoveComponent.h"
#include "Math.h"

class PlayerMove : public MoveComponent
{
public:
    PlayerMove(class Actor *owner);
    void Update(float deltaTime) override;
    void ProcessInput(const Uint8 *keyState) override;
    const float speed = 300.0f;
    float mYspeed = 0;
    const float G = 2000.0f;
    const float bottom = 448.0f;
    const float jumpSpeed = -700.0f;
    const float win = 6368.0f;
    bool mSpacePressed = false;
    bool mInAir = false;
};