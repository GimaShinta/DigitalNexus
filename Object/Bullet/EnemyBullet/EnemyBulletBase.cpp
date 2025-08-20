#include "EnemyBulletBase.h"
#include "../../Item/Exp/Exp2.h"


EnemyBulletBase::EnemyBulletBase()
{
}

EnemyBulletBase::~EnemyBulletBase()
{
}

void EnemyBulletBase::Initialize()
{
}

void EnemyBulletBase::Update(float delata_second)
{
}

void EnemyBulletBase::Draw(const Vector2D& screen_offset) const
{
}

void EnemyBulletBase::Finalize()
{
}

void EnemyBulletBase::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::ePlayer)
	{
		is_destroy = true;
	}

	if (hit_object->GetCollision().object_type == eObjectType::eDefenceShot)
	{
		DropItems();
		is_destroy = true;
	}
}

void EnemyBulletBase::SetPlayer(Player* p)
{
	player = p;
}

void EnemyBulletBase::DropItems()
{
	GameObjectManager* manager = Singleton<GameObjectManager>::GetInstance();

	// ExpiŠm’èj
	Exp2* exp = manager->CreateObject<Exp2>(location);
	exp->SetPlayer(player);
}
