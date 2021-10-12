#include "Goomba.h"
#include "SpriteComponent.h"
#include "MoveComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Random.h"
#include "Math.h"
#include "GoombaMove.h"
#include "AnimatedSprite.h"

Goomba::Goomba(Game *game, const Vector2 &position) : Actor(game)
{
    spriteComponent = new AnimatedSprite(this, 150);
    std::vector<SDL_Texture *> walkAnim{
        GetGame()->GetTexture("Assets/Goomba/Walk0.png"),
        GetGame()->GetTexture("Assets/Goomba/Walk1.png")};
    std::vector<SDL_Texture *> dead{
        GetGame()->GetTexture("Assets/Goomba/Dead.png")};
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("walk", walkAnim);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("dead", dead);
    static_cast<AnimatedSprite *>(spriteComponent)->SetAnimation("walk");
    SetPosition(position);
    SetRotation(0);
    collisionComponent = new CollisionComponent(this, mGame->gridSize, mGame->gridSize);
    new GoombaMove(this);
    mGame->AddGoomba(this);
}

Goomba::~Goomba()
{
    mGame->RemoveGoomba(this);
}

void Goomba::OnProcessInput(const Uint8 *keyState)
{
}

void Goomba::OnUpdate(float deltaTime)
{
    if (stomped)
    {
        static_cast<AnimatedSprite *>(spriteComponent)->SetAnimation("dead");
        timer += deltaTime;
        if (timer >= 0.25f)
        {
            SetState(ActorState::Destroy);
            return;
        }
    }
}
