#include "EnemyBullet2.h"
#include "../../GameObjectManager.h"
#include "../../../Utility/ProjectConfig.h"

EnemyBullet2::EnemyBullet2()
{
}

EnemyBullet2::~EnemyBullet2()
{
}

// 初期化処理
void EnemyBullet2::Initialize()
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

	wave_amplitude = 30.0f; // 初期振れ幅
	wave_frequency = 4.0f;  // 初期周波数

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime81.png", 6, 6, 1, 24, 24)[0];

}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void EnemyBullet2::Update(float delta_second)
{
	move_time += delta_second;

#if 0
	switch (shot_state)
	{
	case ShotState::SpreadOut:
		location += velocity * delta_second;
		if (move_time > 1.0f)
		{
			shot_state = ShotState::PullIn;
			move_time = 0.0f;

			// 中心に吸い込まれる（suck_centerに向かう）
			velocity = (suck_center - location).Normalize() * 100.0f;
		}
		break;

	case ShotState::PullIn:
		location += velocity * delta_second;

		// 中心にほぼ近づいたら次のステートへ
		if ((suck_center - location).Length() < 5.0f)
		{
			shot_state = ShotState::Explode;
			move_time = 0.0f;

			// ランダムに弾ける
			float angle = static_cast<float>(GetRand(359));
			velocity = Vector2D(cosf(angle * DX_PI / 180.0f), sinf(angle * DX_PI / 180.0f)) * 200.0f;
		}
		break;

	case ShotState::Explode:
		location += velocity * delta_second;
		break;
	}

#else
	base_velocity = velocity;
	// 波の軌道を加える
	float offset = sinf(move_time * wave_frequency * 2 * DX_PI) * wave_amplitude;
	if (is_wave_reflected)
		offset *= -1.0f;
	Vector2D sideway = Vector2D(-base_velocity.y, base_velocity.x).Normalize(); // 直角方向
	location += base_velocity * delta_second + sideway * offset * delta_second;


#endif

	// 弾が画面外に出たら削除（移動方向によって判定を変える）
	if (location.y < -300.0f || // 上方向
		location.y > D_WIN_MAX_Y + 300.0f ||
		location.x > D_WIN_MAX_X ||
		location.x < 0 - 350.0f) // 下方向
	{
		is_destroy = true;
	}
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBullet2::Draw(const Vector2D& screen_offset) const
{
	float angle = move_time * 10.0f; // 回転速度
	float hw = box_size.x;
	float hh = box_size.y;

	float cosA = cos(angle);
	float sinA = sin(angle);

	Vector2D p1(-hw, -hh);
	Vector2D p2(hw, -hh);
	Vector2D p3(hw, hh);
	Vector2D p4(-hw, hh);

	auto rotate = [&](Vector2D p) {
		return Vector2D(
			location.x + p.x * cosA - p.y * sinA,
			location.y + p.x * sinA + p.y * cosA
		);
		};

	p1 = rotate(p1);
	p2 = rotate(p2);
	p3 = rotate(p3);
	p4 = rotate(p4);

	int color = GetColor(255, 255, 0);
	//DrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color, TRUE);
	//DrawTriangle(p1.x, p1.y, p3.x, p3.y, p4.x, p4.y, color, TRUE);

	DrawRotaGraph(location.x, location.y, 1.9f, angle, image, TRUE);

}

// 終了時処理
void EnemyBullet2::Finalize()
{
}

void EnemyBullet2::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::ePlayer ||
		hit_object->GetCollision().object_type == eObjectType::eDefenceShot)
	{
		is_destroy = true;
	}

}

void EnemyBullet2::SetSuckCenter(const Vector2D& center)
{
	suck_center = center;
}

void EnemyBullet2::SetWaveParameters(float amplitude, float frequency)
{
	wave_amplitude = amplitude;
	wave_frequency = frequency;
}

void EnemyBullet2::SetWaveReflected(bool reflected)
{
	is_wave_reflected = reflected;
}
