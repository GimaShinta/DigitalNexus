#include "PlayerDefenceBullet.h"

PlayerDefenceBullet::PlayerDefenceBullet()
{
}

PlayerDefenceBullet::~PlayerDefenceBullet()
{
}

void PlayerDefenceBullet::Initialize()
{
	z_layer = 3;
	velocity.y = -1500.0f;
	box_size = Vector2D(5.0f, 14.0f);

	// 当たり判定のオブジェクト設定
	collision.is_blocking = true;
	// 自分のオブジェクトタイプ
	collision.object_type = eObjectType::eDefenceShot;
	// 当たる相手のオブジェクトタイプ
	collision.hit_object_type.push_back(eObjectType::eEnemy);

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Player/Shot/anime_effect17.png", 6, 6, 1, 8, 88)[0];

}

void PlayerDefenceBullet::Finalize()
{
}

void PlayerDefenceBullet::OnHitCollision(GameObjectBase* hit_object)
{
	// すでに吸収済みなら無視
	if (is_collected) return;

	EffectManager* am = Singleton<EffectManager>::GetInstance();
	SEManager* sm = Singleton<SEManager>::GetInstance();
	int anim_id = 0;
	// 当たった相手が弾だったら
	if (hit_object->GetCollision().object_type == eObjectType::eEnemyShot)
	{
		is_collected = true;

		float random_x = static_cast<float>(GetRand(20));
		if (GetRand(2) == 1)
		{
			sm->PlaySE(SE_NAME::Hit);
			sm->ChangeSEVolume(SE_NAME::Hit, 50);
			//PlaySoundMem(hit_se, DX_PLAYTYPE_BACK);
			random_x *= -1;
		}
		float random_y = static_cast<float>(GetRand(15));

		anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x - random_x, location.y - random_y), 0.01f, false);
		// アニメーションの追加設定
		am->SetAlpha(anim_id, 255);       // 半透明
		am->SetScale(anim_id, 0.2f);      // 1.5倍拡大

		Singleton<ScoreData>::GetInstance()->AddScore(50);
		Singleton<ShakeManager>::GetInstance()->StartShake(0.5, 3, 3);

		is_destroy = true;
	}
	//if (hit_object->GetCollision().object_type == eObjectType::eBoss)
	//{
	//	sm->PlaySE(SE_NAME::Hit);
	//	sm->ChangeSEVolume(SE_NAME::Hit, 50);
	//	float random_x = static_cast<float>(GetRand(100));

	//	anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x, location.y - random_x), 0.01f, false);
	//	// アニメーションの追加設定
	//	am->SetAlpha(anim_id, 255);       // 半透明
	//	am->SetScale(anim_id, 0.2f);      // 1.5倍拡大

	//	is_destroy = true;
	//}
}
