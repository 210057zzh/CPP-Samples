#include "CollisionComponent.h"
#include "Actor.h"
#include <functional>

CollisionComponent::CollisionComponent(class Actor *owner)
	: Component(owner), mWidth(0.0f), mHeight(0.0f)
{
}

CollisionComponent::CollisionComponent(class Actor *owner, float width, float height)
	: Component(owner), mWidth(width), mHeight(height)
{
}

CollisionComponent::~CollisionComponent()
{
}

bool CollisionComponent::Intersect(const CollisionComponent *other)
{
	const auto aMin = GetMin();
	const auto aMax = GetMax();
	const auto bMin = other->GetMin();
	const auto bMax = other->GetMax();
	bool case1 = aMax.x < bMin.x;
	bool case2 = bMax.x < aMin.x;
	bool case3 = aMax.y < bMin.y;
	bool case4 = bMax.y < aMin.y;
	return !(case1 || case2 || case3 || case4);
}

Vector2 CollisionComponent::GetMin() const
{
	auto position = GetCenter();
	return Vector2(position.x - mWidth / 2.0f, position.y - mHeight / 2.0f);
}

Vector2 CollisionComponent::GetMax() const
{
	auto position = GetCenter();
	return Vector2(position.x + mWidth / 2.0f, position.y + mHeight / 2.0f);
}

const Vector2 &CollisionComponent::GetCenter() const
{
	return mOwner->GetPosition();
}

CollSide CollisionComponent::GetMinOverlap(
	const CollisionComponent *other, Vector2 &offset)
{
	offset = Vector2::Zero;
	if (!Intersect(other))
		return CollSide::None;
	auto thisMin = GetMin();
	auto thisMax = GetMax();
	auto otherMin = other->GetMin();
	auto otherMax = other->GetMax();
	auto otherMaxYDiff = Math::Abs(otherMax.y - thisMin.y);
	auto otherMinYDiff = Math::Abs(otherMin.y - thisMax.y);
	auto otherMaxXDiff = Math::Abs(otherMax.x - thisMin.x);
	auto otherMinXDiff = Math::Abs(otherMin.x - thisMax.x);
	auto min = Math::Min(otherMaxYDiff, Math::Min(otherMinYDiff, Math::Min(otherMaxXDiff, otherMinXDiff)));
	if (min == otherMaxYDiff)
	{
		offset.y += otherMaxYDiff;
		return CollSide::Bottom;
	}
	else if (min == otherMinYDiff)
	{
		offset.y -= otherMinYDiff;
		return CollSide::Top;
	}
	else if (min == otherMaxXDiff)
	{
		offset.x += otherMaxXDiff;
		return CollSide::Right;
	}
	else //otherMinXDiff
	{
		offset.x -= otherMinXDiff;
		return CollSide::Left;
	}
}
