#include "Block.h"
#include "SpriteComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Random.h"
#include "Math.h"

std::unordered_map<char, std::string> Block::filenames;

Block::Block(Game *game, const Vector2 &position, char type) : Actor(game)
{
    mGame->AddBlock(this);
    spriteComponent = new SpriteComponent(this);
    if (filenames.find(type) != filenames.end())
    {
        spriteComponent->SetTexture(game->GetTexture(filenames[type]));
    }
    else
    {
        std::string newName = prefix + type + extention;
        spriteComponent->SetTexture(game->GetTexture(newName));
        filenames.emplace(type, std::move(newName));
    }
    collisionComponent = new CollisionComponent(this, mGame->gridSize, mGame->gridSize);
    SetPosition(position);
    SetRotation(0);
}

Block::~Block()
{
    mGame->RemoveBlock(this);
}

void Block::OnUpdate(float deltaTime)
{
}
