#include "EnemyBullet5.h"

EnemyBullet5::EnemyBullet5()
{
}

EnemyBullet5::~EnemyBullet5()
{
}

void EnemyBullet5::Initialize()
{
	z_layer = 4;
	box_size = 12;

	// 当たり判定のオブジェクト設定
	collision.is_blocking = true;
	// 自分のオブジェクトタイプ
	collision.object_type = eObjectType::eEnemyShot;
	// 当たる相手のオブジェクトタイプ
	collision.hit_object_type.push_back(eObjectType::ePlayer);
	collision.hit_object_type.push_back(eObjectType::eDefenceShot);

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime81.png", 6, 6, 1, 24, 24)[0];

}

void EnemyBullet5::Update(float delta_second)
{
	// 弾が画面外に出たら削除（移動方向によって判定を変える）
	if (location.y < -50.0f || // 上方向
		location.y > D_WIN_MAX_Y + 50.0f ||
		location.x > 990.0f ||
		location.x < 240.0f) // 下方向
	{
		is_destroy = true;
	}


	timer += delta_second;

	float offset = sinf(timer * frequency) * amplitude;
	if (reverse) offset *= -1;

	// 横に波打つように進行
	Vector2D velocity = base_velocity + Vector2D(offset, 0.0f);

	location += velocity * delta_second;
}

void EnemyBullet5::Draw(const Vector2D& screen_offset) const
{
	DrawRotaGraph(location.x, location.y, 1.9f, 0.0f, image, TRUE);
}

void EnemyBullet5::Finalize()
{
}

void EnemyBullet5::OnHitCollision(GameObjectBase* hit_object)
{
}

void EnemyBullet5::SetWaveParameters(const Vector2D& base_velocity, float amplitude, float frequency, bool reverse)
{
	this->base_velocity = base_velocity;
	this->amplitude = amplitude;
	this->frequency = frequency;
	this->reverse = reverse;
	this->timer = 0.0f;
}
