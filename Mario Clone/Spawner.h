#pragma once
#include "Actor.h"

class Spawner : public Actor
{
public:
    Spawner(class Game *game, const Vector2 &position);
    ~Spawner();
    virtual void OnUpdate(float deltaTime) override;

private:
    bool up;
    bool down;
    bool left;
    bool right;
    const float forwardSpeed = 150;
    const float angularSpeed = 0;
};