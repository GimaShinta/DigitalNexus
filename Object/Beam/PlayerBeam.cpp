#include "PlayerBeam.h"
#include "../Character/Player/Player.h"

PlayerBeam::PlayerBeam()
{
}

PlayerBeam::~PlayerBeam()
{
}

void PlayerBeam::Initialize()
{
	z_layer = 2;
	box_size = Vector2D(60.0f, 800.0f);

	// 当たり判定のオブジェクト設定
	collision.is_blocking = true;
	// 自分のオブジェクトタイプ
	collision.object_type = eObjectType::eBeam;
	// 当たる相手のオブジェクトタイプ
	collision.hit_object_type.push_back(eObjectType::eEnemy);
	collision.hit_object_type.push_back(eObjectType::eBoss);

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	beam_ts = rm->GetImages("Resource/Image/Object/Player/Beam/anime_sp_weapon03_1.png", 12, 2, 6, 88, 128);
	beam_t = beam_ts[10];
	beam_bs = rm->GetImages("Resource/Image/Object/Player/Beam/anime_sp_weapon03_2.png", 12, 2, 6, 88, 80);
	beam_b = beam_bs[10];

	beam_state = BeamState::Charging;
	charge_time = 0.0f;
}

void PlayerBeam::Update(float delta_second)
{
	switch (beam_state)
	{
	case BeamState::Charging:
		charge_time += delta_second;

		if (charge_time >= charge_duration)
		{
			beam_state = BeamState::Firing;
		}
		return; // チャージ中はビームの位置などを更新しない
	case BeamState::Firing:
		break; // ↓ 既存の処理へ
	default:
		return;
	}

	if (player != nullptr)
	{
		location.x = player->GetLocation().x;
		if (Beam_flip == false)
		{
			location.y = (player->GetLocation().y - (D_OBJECT_SIZE + 12.0f)) - box_size.y;
		}
		else
		{
			location.y = (player->GetLocation().y + D_OBJECT_SIZE) + box_size.y;
		}
	}

	destroy_time += delta_second;

	if (destroy_time >= 5.0f)
	{
		is_destroy = true;
	}

	std::vector<int> animation_num = { 10,11 };
	//フレームレートで時間を計測
	animation_time += delta_second;
	//8秒経ったら画像を切り替える
	if (animation_time >= 0.03f)
	{
		//計測時間の初期化
		animation_time = 0.0f;
		//時間経過カウントの増加
		animation_count++;
		//カウントがアニメーション画像の要素数以上になったら
		if (animation_count >= animation_num.size())
		{
			//カウントの初期化
			animation_count = 0;
		}
		// アニメーションが順番に代入される
		beam_t = beam_ts[animation_num[animation_count]];
		beam_b = beam_bs[animation_num[animation_count]];
	}

	location += velocity * delta_second;
}

void PlayerBeam::Draw(const Vector2D& screen_offset) const
{
	if (beam_state != BeamState::Firing) return;

	DrawRotaGraph(location.x, (location.y + box_size.y) - 600.0f, 1.5f, 0.0f, beam_t, TRUE); // トップ
	DrawRotaGraph(location.x, (location.y + box_size.y) - 500.0f, 1.5f, 0.0f, beam_t, TRUE); // トップ
	DrawRotaGraph(location.x, (location.y + box_size.y) - 400.0f, 1.5f, 0.0f, beam_t, TRUE); // トップ
	DrawRotaGraph(location.x, (location.y + box_size.y) - 300.0f, 1.5f, 0.0f, beam_t, TRUE); // トップ
	DrawRotaGraph(location.x, (location.y + box_size.y) - 200.0f, 1.5f, 0.0f, beam_t, TRUE); // トップ
	DrawRotaGraph(location.x, (location.y + box_size.y) - 50.0f, 1.5f, 0.0f, beam_b, TRUE); // ボトム
}

void PlayerBeam::Finalize()
{
}

void PlayerBeam::OnHitCollision(GameObjectBase* hit_object)
{
	// 0.5秒未満なら爆発生成しない
	if (destroy_time - last_explosion_time < 0.1f)
	{
		return;
	}
	last_explosion_time = destroy_time;

	//if (hit_object->GetCollision().object_type == eObjectType::eBoss3)
	//{
	//	EffectManager* am = Singleton<EffectManager>::GetInstance();
	//	int anim_id = 0;

	//	float random_x = static_cast<float>(GetRand(70));
	//	if (GetRand(2) == 1)
	//	{
	//		random_x *= -1;
	//	}
	//	Vector2D t_location = hit_object->GetLocation();

	//	float random_y = static_cast<float>(GetRand(100));

	//	anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x + random_x, (t_location.y + hit_object->GetBoxSize().y) - random_y), 0.03f, false);
	//	// アニメーションの追加設定
	//	am->SetAlpha(anim_id, 255);       // 半透明
	//	am->SetScale(anim_id, 0.8f);      // 1.5倍拡大
	//	am->SetZLayer(anim_id, 3);
	//}
	//else
	{
		EffectManager* am = Singleton<EffectManager>::GetInstance();
		int anim_id = 0;

		float random_x = static_cast<float>(GetRand(70));
		if (GetRand(2) == 1)
		{
			random_x *= -1;
		}
		Vector2D t_location = hit_object->GetLocation();

		float random_y = static_cast<float>(GetRand(150));

		anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x + random_x, (t_location.y + 48.0f) - random_y), 0.03f, false);
		// アニメーションの追加設定
		am->SetAlpha(anim_id, 255);       // 半透明
		am->SetScale(anim_id, 0.8f);      // 1.5倍拡大
		am->SetZLayer(anim_id, 3);
	}
}

void PlayerBeam::SetPlayer(Player* player)
{
	player = player;
}
