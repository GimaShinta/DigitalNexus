#include "EnemyBullet1.h"
#include "../../GameObjectManager.h"
#include "../../../Utility/ProjectConfig.h"

EnemyBullet1::EnemyBullet1()
{
}

EnemyBullet1::~EnemyBullet1()
{
}

// 初期化処理
void EnemyBullet1::Initialize()
{
	z_layer = 4;
	box_size = 6;

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
	image = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/standard/bullet73.png")[0];
	se = rm->GetSounds("Resource/sound/se/shot/shot_01.mp3");
	ChangeVolumeSoundMem(255 * 90 / 100, se);
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void EnemyBullet1::Update(float delta_second)
{
	move_time += delta_second;

	// 弾が画面外に出たら削除（移動方向によって判定を変える）
	if (location.y < -300.0f || // 上方向
		location.y > D_WIN_MAX_Y + 300.0f ||
		location.x > (D_WIN_MAX_X / 2) + 350.0f ||
		location.x < (D_WIN_MAX_X / 2) - 350.0f) // 下方向
	{
		is_destroy = true;
	}

	location += (velocity * 200) * delta_second;
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBullet1::Draw(const Vector2D& screen_offset) const
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

	PlaySoundMem(se, DX_PLAYTYPE_BACK);
	DrawRotaGraph(location.x, location.y, 1.9f, angle, image, TRUE);

}

// 終了時処理
void EnemyBullet1::Finalize()
{
}

void EnemyBullet1::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::ePlayer ||
		hit_object->GetCollision().object_type == eObjectType::eDefenceShot)
	{
		is_destroy = true;
	}
}
