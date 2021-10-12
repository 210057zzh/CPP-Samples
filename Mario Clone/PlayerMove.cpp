#include "PlayerMove.h"
#include "Actor.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "Block.h"
#include "Goomba.h"
#include "AnimatedSprite.h"

PlayerMove::PlayerMove(class Actor *owner) : MoveComponent(owner)
{
}

void PlayerMove::Update(float deltaTime)
{
    auto newPosition = mOwner->GetPosition() + GetForwardSpeed() * deltaTime * mOwner->GetForward();
    newPosition.y += mYspeed * deltaTime;
    auto animComp = mOwner->GetComponent<AnimatedSprite>();
    if (newPosition.y > bottom)
    {
        animComp->SetAnimation("dead");
        mOwner->SetState(ActorState::Paused);
        Mix_HaltChannel(mOwner->GetGame()->theme);
        Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/Dead.wav"), 0);
        return;
    }
    else if (newPosition.x > win)
    {
        Mix_HaltChannel(mOwner->GetGame()->theme);
        mOwner->SetState(ActorState::Paused);
        Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/StageClear.wav"), 0);
    }
    mOwner->SetPosition(newPosition);
    bool onBlock = false;
    auto collComp = mOwner->GetComponent<CollisionComponent>();
    for (auto &block : mOwner->GetGame()->GetBlocks())
    {
        Vector2 offset;
        auto collSide = collComp->GetMinOverlap(block->collisionComponent, offset);
        if (collSide == CollSide::None)
            continue;
        onBlock = true;
        auto position = mOwner->GetPosition();
        if (collSide == CollSide::Top && mYspeed > 0.0f)
        {
            mYspeed = 0.0f;
            mInAir = false;
        }
        else if (collSide == CollSide::Bottom && mYspeed < 0)
        {
            mYspeed = 0;
            Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/Bump.wav"), 0);
        }
        mOwner->SetPosition(position + offset);
    }
    for (auto &goomba : mOwner->GetGame()->GetGoombas())
    {
        if (!goomba->stomped)
        {
            Vector2 offset;
            auto collSide = collComp->GetMinOverlap(goomba->collisionComponent, offset);
            if (collSide == CollSide::None)
                continue;
            auto position = mOwner->GetPosition();
            switch (collSide)
            {
            case CollSide::Top:
                goomba->stomped = true;
                mYspeed = jumpSpeed / 2.0f;
                mInAir = true;
                break;
            case CollSide::Left:
            case CollSide::Right:
                if (mInAir)
                {
                    goomba->stomped = true;
                    mYspeed = jumpSpeed / 2.0f;
                    mInAir = true;
                }
                else
                {
                    animComp->SetAnimation("dead");
                    mOwner->SetState(ActorState::Paused);
                    Mix_HaltChannel(mOwner->GetGame()->theme);
                    Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/Dead.wav"), 0);
                    return;
                }
                break;
            default:
                break;
            }
            if (goomba->stomped)
                Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/Stomp.wav"), 0);
            mOwner->SetPosition(position + offset);
        }
    }
    if (!onBlock)
        mInAir = true;
    mYspeed += G * deltaTime;
    auto position = mOwner->GetPosition();
    auto &cameraPos = mOwner->GetGame()->GetCameraPos();
    if (position.x - mOwner->GetGame()->windowWidth / 2.0f > cameraPos.x)
        cameraPos.x = position.x - mOwner->GetGame()->windowWidth / 2.0f;
    if (cameraPos.x < 0)
        cameraPos.x = 0;
    if (position.x < cameraPos.x)
        position.x = cameraPos.x;
    mOwner->SetPosition(position);
    auto currentSpeed = GetForwardSpeed();
    if (mInAir)
    {
        if (currentSpeed < 0) //left
        {
            animComp->SetAnimation("jumpLeft");
        }
        else if (currentSpeed > 0) //right
        {
            animComp->SetAnimation("jumpRight");
        }
        else
        {
            auto &animName = animComp->GetAnimName();
            if (animName == "runRight" || animName == "jumpRight" || animName == "idle")
            {
                animComp->SetAnimation("jumpRight");
            }
            else
            {
                animComp->SetAnimation("jumpLeft");
            }
        }
    }
    else
    {
        if (currentSpeed < 0) //left
        {
            animComp->SetAnimation("runLeft");
        }
        else if (currentSpeed > 0) //right
        {
            animComp->SetAnimation("runRight");
        }
        else
        {
            animComp->SetAnimation("idle");
        }
    }
}

void PlayerMove::ProcessInput(const Uint8 *keyState)
{
    if ((keyState[SDL_SCANCODE_LEFT] && keyState[SDL_SCANCODE_RIGHT]) || !(keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_RIGHT]))
    {
        SetForwardSpeed(0);
    }
    else if (keyState[SDL_SCANCODE_LEFT])
    {
        SetForwardSpeed(-1 * speed);
    }
    else if (keyState[SDL_SCANCODE_RIGHT])
    {
        SetForwardSpeed(speed);
    }
    if (keyState[SDL_SCANCODE_SPACE] && !mSpacePressed && !mInAir)
    {
        mYspeed = jumpSpeed;
        mInAir = true;
        Mix_PlayChannel(-1, mOwner->GetGame()->GetSound("Assets/Sounds/Jump.wav"), 0);
    }
    mSpacePressed = keyState[SDL_SCANCODE_SPACE];
}
