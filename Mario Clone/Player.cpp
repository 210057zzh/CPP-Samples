#include "Player.h"
#include "AnimatedSprite.h"
#include "MoveComponent.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "Random.h"
#include "Math.h"
#include "PlayerMove.h"

Player::Player(Game *game, const Vector2 &position) : Actor(game)
{
    spriteComponent = new AnimatedSprite(this, 200);
    std::vector<SDL_Texture *> idle{
        GetGame()->GetTexture("Assets/Mario/Idle.png")};
    std::vector<SDL_Texture *> dead{
        GetGame()->GetTexture("Assets/Mario/Dead.png")};
    std::vector<SDL_Texture *> jumpLeft{
        GetGame()->GetTexture("Assets/Mario/JumpLeft.png")};
    std::vector<SDL_Texture *> jumpRight{
        GetGame()->GetTexture("Assets/Mario/JumpRight.png")};
    std::vector<SDL_Texture *> runLeft{
        GetGame()->GetTexture("Assets/Mario/RunLeft0.png"),
        GetGame()->GetTexture("Assets/Mario/RunLeft1.png"),
        GetGame()->GetTexture("Assets/Mario/RunLeft2.png")};
    std::vector<SDL_Texture *> runRight{
        GetGame()->GetTexture("Assets/Mario/RunRight0.png"),
        GetGame()->GetTexture("Assets/Mario/RunRight1.png"),
        GetGame()->GetTexture("Assets/Mario/RunRight2.png")};
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("idle", idle);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("dead", dead);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("jumpLeft", jumpLeft);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("jumpRight", jumpRight);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("runLeft", runLeft);
    static_cast<AnimatedSprite *>(spriteComponent)->AddAnimation("runRight", runRight);
    static_cast<AnimatedSprite *>(spriteComponent)->SetAnimation("idle");
    SetPosition(position);
    SetRotation(0);
    collisionComponent = new CollisionComponent(this, mGame->gridSize, mGame->gridSize);
    new PlayerMove(this);
}

Player::~Player()
{
}

void Player::OnProcessInput(const Uint8 *keyState)
{
}

void Player::OnUpdate(float deltaTime)
{
}
