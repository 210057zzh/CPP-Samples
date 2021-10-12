#include "AnimatedSprite.h"
#include "Actor.h"
#include "Game.h"
#include <cmath>

AnimatedSprite::AnimatedSprite(Actor *owner, int drawOrder)
	: SpriteComponent(owner, drawOrder)
{
}

void AnimatedSprite::Update(float deltaTime)
{
	// TODO: Implement
	if (!mAnimName.empty())
	{
		if (!mIsPaused)
		{
			mAnimTimer = Math::Fmod((mAnimTimer + deltaTime * mAnimFPS), static_cast<float>(mAnims[mAnimName].size()));
		}
		SetTexture(mAnims[mAnimName][(size_t)mAnimTimer]);
	}
}

void AnimatedSprite::SetAnimation(const std::string &name, bool resetTimer)
{
	if (mAnimName != name)
	{
		mAnimName = name;
	}

	if (resetTimer)
	{
		mAnimTimer = 0.0f;
	}
}

void AnimatedSprite::AddAnimation(const std::string &name,
								  const std::vector<SDL_Texture *> &images)
{
	mAnims.emplace(name, images);
}
