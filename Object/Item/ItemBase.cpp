#include "ItemBase.h"

void ItemBase::Initialize()
{
}

void ItemBase::Update(float delata_second)
{
}

void ItemBase::Draw(const Vector2D& screen_offset) const
{
}

void ItemBase::Finalize()
{
}

void ItemBase::OnHitCollision(GameObjectBase* hit_object)
{
}

void ItemBase::SetPlayer(Player* p)
{
	player = p;
}
