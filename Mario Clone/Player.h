#pragma once
#include "Actor.h"

class Player : public Actor
{
public:
    Player(class Game *game, const Vector2 &position);
    ~Player();
    class MoveComponent *moveComponent;
    class SpriteComponent *spriteComponent;
    class CollisionComponent *collisionComponent;
    virtual void OnProcessInput(const Uint8 *keyState);
    virtual void OnUpdate(float deltaTime) override;

private:
    bool up;
    bool down;
    bool left;
    bool right;
    const float forwardSpeed = 150;
    const float angularSpeed = 0;
};