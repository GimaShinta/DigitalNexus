#include "EnemyBase.h"
#include"../../../Utility/ScoreData.h"
#include "../../Item/Exp/Exp.h"
#include "../../Item/PowerUp/PowerUp.h"
#include "../../Item/Shield/Shield.h"
#include "../../Item/MaxCharge/MaxCharge.h"

EnemyBase::EnemyBase() : hp(0.0f), on_hit(false)
{
}

EnemyBase::~EnemyBase()
{
}

// 初期化処理
void EnemyBase::Initialize()
{
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void EnemyBase::Update(float delta_second)
{
	delta = delta_second;

	// 親クラスの更新処理を呼び出す
	__super::Update(delta_second);
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBase::Draw(const Vector2D& screen_offset) const
{
	// 親クラスの描画処理を呼び出す
	__super::Draw(screen_offset);
}

// 終了時処理
void EnemyBase::Finalize()
{
}

/// <summary>
/// ヒット時処理
/// </summary>
/// <param name="hit_object">当たった相手</param>
void EnemyBase::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::eShot)
	{
		if (on_hit == false)
		{
			hp -= 10;
			on_hit = true;
		}
		else
		{
			on_hit = false;
		}
	}
	if (hit_object->GetCollision().object_type == eObjectType::eBeam)
	{
		beam_damage_timer += delta;

		if (beam_damage_timer >= 0.05f)
		{
			hp -= 10;
			beam_damage_timer = 0;
		}
	}
}

void EnemyBase::SetPlayer(Player* p)
{
	player = p;
}

void EnemyBase::DropItems()
{
	//auto& manager = *Singleton<GameObjectManager>::GetInstance();

	//// 100%でExpはドロップ
	//auto exp = manager.CreateObject<Exp>(location);
	//exp->SetPlayer(player);

	////1%でPowerUpドロップ
	//if (rand() % 100 < 2) {
	//	auto charge = manager.CreateObject<MaxCharge>(location);
	//	charge->SetPlayer(player);
	//}

	//// 5%でShieldドロップ
	//if (rand() % 100 < 2) {
	//	auto shi = manager.CreateObject<Shield>(location);
	//	shi->SetPlayer(player);

	//}
}
