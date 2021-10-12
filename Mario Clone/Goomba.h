#pragma once
#include "Actor.h"

class Goomba : public Actor
{
public:
    Goomba(class Game *game, const Vector2 &position);
    ~Goomba();
    class MoveComponent *moveComponent;
    class SpriteComponent *spriteComponent;
    class CollisionComponent *collisionComponent;
    virtual void OnProcessInput(const Uint8 *keyState);
    virtual void OnUpdate(float deltaTime) override;
    bool stomped = false;

private:
    const float forwardSpeed = 150;
    const float angularSpeed = 0;
    float timer = 0.0f;
};