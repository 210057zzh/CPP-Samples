#include "GoombaMove.h"
#include "Actor.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "Block.h"
#include "Goomba.h"

GoombaMove::GoombaMove(class Actor *owner) : MoveComponent(owner)
{
    SetForwardSpeed(-1.0f * speed);
}

void GoombaMove::Update(float deltaTime)
{
    if (!(static_cast<Goomba *>(mOwner))->stomped)
    {
        auto newPosition = mOwner->GetPosition() + GetForwardSpeed() * deltaTime * mOwner->GetForward();
        newPosition.y += mYspeed * deltaTime;
        if (newPosition.y > bottom)
        {
            mOwner->SetState(ActorState::Destroy);
            return;
        }
        mOwner->SetPosition(newPosition);
        mInAir = checkCollisionsWithCollection(mOwner->GetGame()->GetBlocks());
        checkCollisionsWithCollection(mOwner->GetGame()->GetGoombas());
        mYspeed += G * deltaTime;
    }
}

void GoombaMove::ProcessInput(const Uint8 *keyState)
{
}

template <typename t>
bool GoombaMove::checkCollisionsWithCollection(std::vector<t> &collection)
{
    bool onBlock = false;
    bool ret = false;
    auto collComp = mOwner->GetComponent<CollisionComponent>();
    for (auto &block : collection)
    {
        if (block != mOwner)
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
                ret = false;
            }
            else if (collSide == CollSide::Bottom && mYspeed < 0)
            {
                mYspeed = 0;
            }
            else if (collSide == CollSide::Right && GetForwardSpeed() < 0 && Math::Abs(position.y - block->GetPosition().y) < 10)
            {
                SetForwardSpeed(speed);
            }
            else if (collSide == CollSide::Left && GetForwardSpeed() > 0 && Math::Abs(position.y - block->GetPosition().y) < 10)
            {
                SetForwardSpeed(-1.0f * speed);
            }
            mOwner->SetPosition(position + offset);
        }
    }
    if (!onBlock)
        ret = true;
    return ret;
}
