#include "Spawner.h"
#include "MoveComponent.h"
#include "Game.h"
#include "Random.h"
#include "Math.h"
#include "Goomba.h"

Spawner::Spawner(Game *game, const Vector2 &position) : Actor(game)
{
    SetPosition(position);
    SetRotation(0);
}

Spawner::~Spawner()
{
}

void Spawner::OnUpdate(float deltaTime)
{
    if (Math::Abs(GetPosition().x - mGame->GetPlayer()->GetPosition().x < 600.0f))
    {
        new Goomba(mGame, GetPosition());
        SetState(ActorState::Destroy);
    }
}
