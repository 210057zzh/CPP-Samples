#pragma once
#include "Actor.h"
#include "string"
#include <unordered_map>

class Block : public Actor
{
public:
    Block(class Game *game, const Vector2 &position, char type);
    ~Block();
    class SpriteComponent *spriteComponent;
    class CollisionComponent *collisionComponent;
    virtual void OnUpdate(float deltaTime);
    const std::string prefix = "Assets/Block";
    const std::string extention = ".png";
    static std::unordered_map<char, std::string> filenames;

private:
    const float forwardSpeed = 100;
    const float angularSpeed = 0;
};
