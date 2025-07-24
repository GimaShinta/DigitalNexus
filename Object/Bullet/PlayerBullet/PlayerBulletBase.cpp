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
	// 弾が画面外に出たら削除（移動方向によって判定を変える）
	if ((velocity.y < 240 && location.y < -50.0f) || // 上方向
		(velocity.y > 990 && location.y > D_WIN_MAX_Y + 50.0f)) // 下方向
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
