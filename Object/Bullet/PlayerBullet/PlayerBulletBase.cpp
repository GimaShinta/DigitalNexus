#include "PlayerBulletBase.h"

PlayerBulletBase::PlayerBulletBase()
{
}

PlayerBulletBase::~PlayerBulletBase()
{
}

void PlayerBulletBase::Initialize()
{
}

void PlayerBulletBase::Update(float delta_second)
{
	// �e����ʊO�ɏo����폜�i�ړ������ɂ���Ĕ����ς���j
	if ((velocity.y < 240 && location.y < -50.0f) || // �����
		(velocity.y > 990 && location.y > D_WIN_MAX_Y + 50.0f)) // ������
	{
		is_destroy = true;
	}

	location += velocity * delta_second;
}

void PlayerBulletBase::Draw(const Vector2D& screen_offset) const
{
	DrawRotaGraph(location.x, location.y, 1.1f, 0.0f, image, TRUE);
}

void PlayerBulletBase::Finalize()
{
}

void PlayerBulletBase::OnHitCollision(GameObjectBase* hit_object)
{
}
