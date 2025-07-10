#include "Boss3.h"
#include "../../../Utility/ProjectConfig.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/SEManager.h"
#include "../Player/Player.h"
#include "../../Bullet/EnemyBullet3.h"
#include "../../Bullet/EnemyBullet4.h"
#include "../../Beam/EnemyBeam.h"

// 線形補間
float Lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

Boss3::Boss3()
{
}

Boss3::~Boss3()
{
}

// 初期化処理
void Boss3::Initialize()
{
	z_layer = 3;
	box_size = 30;
	hp = 100000;

	// 攻撃パターンの設定
	attack_pattrn_num = { 7, 12, 5, 12, 6, 12, 10, 12 };


	// 当たり判定のオブジェクト設定
	collision.is_blocking = false;
	collision.object_type = eObjectType::eNone;
	collision.hit_object_type.clear();

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	// 戦闘中の中心座標
	base_position = Vector2D(D_WIN_MAX_X / 2, (D_WIN_MAX_Y / 2) - 100);

	// 登場時の中心座標
	generate_base_position = Vector2D(D_WIN_MAX_X / 2 + 150, D_WIN_MAX_Y + 200);

	// 初期座標は登場時のものを使用
	location = generate_base_position;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	Boss3_image[0] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_a1.png")[0];
	Boss3_image[1] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_a2.png")[0];
	Boss3_image[2] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_b1.png")[0];
	Boss3_image[3] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_b2.png")[0];
	Boss3_image[4] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_b3.png")[0];
	Boss3_image[5] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_b4.png")[0];
	Boss3_image[6] = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/boss03_b5.png")[0];

	Boss3_anim = rm->GetImages("Resource/Image/Object/Enemy/Boss/Boss_02/anime01.png", 6, 6, 1, 80, 40);
	Boss3_image[7] = Boss3_anim[0];

	Boss3_jet = rm->GetImages("Resource/Image/Effect/exhaust_03_spritesheet.png", 24, 8, 3, 128, 200);
	jet = Boss3_jet[0];

	// 最初は本体の位置に固定
	for (int i = 0; i < 6; ++i)
	{
		part_positions[i] = location;
	}

	for (int i = 0; i < 5; ++i) {
		ripples[i].active = false;
		ripples[i].timer = 0.0f;
	}

	// 例：攻撃パターンごとに左右の波紋位置を定義
	ripple_positions[0] = { Vector2D(-160,  100), Vector2D(160,  100) };
	ripple_positions[7] = { Vector2D(-100,  70), Vector2D(100,  70) };
	ripple_positions[5] = { Vector2D(-100,  70), Vector2D(100,  70) };
	ripple_positions[6] = { Vector2D(-100,  70), Vector2D(100,  70) };
	ripple_positions[10] = { Vector2D(-100,  70), Vector2D(100,  70) };
	ripple_positions[12] = { Vector2D(-160,  100), Vector2D(160,  100) };
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void Boss3::Update(float delta_second)
{
	// アニメーション
	GameObjectBase::AnimationControl(image);

	// 移動処理
	Movement(delta_second);

	// 体力がなくなったら削除
	if (hp <= 0 && !is_crashing) {
		is_crashing = true;
		velocity = Vector2D(0, 0); // 落下は手動処理
		Singleton<ScoreData>::GetInstance()->AddScore(15000);

		return; // このフレームで以降の処理は行わない
	}

	// 部品の相対オフセット（左右に2個ずつ）
	Vector2D offsets_2[6] = {
		Vector2D(-180, 0), // 左奥
		Vector2D(-100,  0), // 左手前
		Vector2D(100,  0), // 右手前
		Vector2D(180, 0),  // 右奥

		Vector2D(-100, -100),  // 砲
		Vector2D(100, -100)  // 砲
	};


	if (is_crashing) {
		// 衝突処理は無効化
		collision.object_type = eObjectType::eNone;
		collision.hit_object_type.clear();

		// タイマー加算
		crash_timer += delta_second;

		const float crash_duration = 10.0f;
		float t = Clamp(crash_timer / crash_duration, 0.0f, 1.0f);
		float eased_t = 1.0f - pow(1.0f - t, 3);
		image_size = Lerp(2.0f, 1.0f, eased_t); // スケール縮小

		// ===== 放物線的な移動 =====
		crash_velocity.y += gravity * delta_second;   // 重力で落下加速
		base_position += crash_velocity * delta_second;

		// ===== 振動演出 =====
		float shake_amplitude = 20.0f * t;
		float shake = sin(crash_timer * 10.0f) * shake_amplitude;
		base_position.y += shake * delta_second;

		// ===== 部品を縮めて本体に寄せる =====
		for (int i = 0; i < 6; ++i) {
			float offset_ratio = Lerp(1.0f, 0.5f, eased_t);
			Vector2D adjusted_offset = offsets_2[i] * offset_ratio;
			part_positions[i] = location + adjusted_offset;
		}

		// ===== 爆発処理開始（初回だけ）=====
		if (!explosions_started) {
			explosions_started = true;
			explosion_index = 0;
			explosion_timer = 0.0f;
			SEManager::GetInstance()->PlaySE(SE_NAME::Bakuhatu);
			SEManager::GetInstance()->PlaySE(SE_NAME::Kill);
			//PlaySoundMem(se[0], DX_PLAYTYPE_BACK);
			//PlaySoundMem(se[1], DX_PLAYTYPE_BACK);
			// 初回の爆発を即時生成
			float offset_x = static_cast<float>(GetRand(200) - 100);
			float offset_y = static_cast<float>(GetRand(200) - 100);
			Vector2D random_pos = location + Vector2D(offset_x, offset_y);
			float scale = 0.3f + (GetRand(200) / 200.0f); // 0.5 ～ 1.5

			int id = EffectManager::GetInstance()->PlayerAnimation(
				EffectName::eExprotion2,
				random_pos,
				0.05f,
				false
			);
			EffectManager::GetInstance()->SetScale(id, scale);

			explosion_index++; // 最初の爆発カウント
		}

		// ===== 時間差で爆発を発生 =====
		if (explosions_started) {
			explosion_timer += delta_second;

			// 複数爆発（最大数＆間隔）
			if (explosion_index < max_explosions && explosion_timer >= explosion_interval) {
				explosion_timer = 0.0f;

				float offset_x = static_cast<float>(GetRand(200) - 100);
				float offset_y = static_cast<float>(GetRand(200) - 100);
				Vector2D random_pos = location + Vector2D(offset_x, offset_y);
				float scale = 0.3f + (GetRand(200) / 200.0f); // 0.5 ～ 1.5
				//PlaySoundMem(se[0], DX_PLAYTYPE_BACK);
				SEManager::GetInstance()->PlaySE(SE_NAME::Bakuhatu);
				int id = EffectManager::GetInstance()->PlayerAnimation(
					EffectName::eExprotion2,
					random_pos,
					0.05f,
					false
				);
				EffectManager::GetInstance()->SetScale(id, scale);

				explosion_index++;
			}

			// 全爆発完了後に大爆発＆削除
			if (explosion_index >= max_explosions) {
				//PlaySoundMem(se[1], DX_PLAYTYPE_BACK);
				//PlaySoundMem(se[2], DX_PLAYTYPE_BACK);
				SEManager::GetInstance()->PlaySE(SE_NAME::Kill);
				SEManager::GetInstance()->PlaySE(SE_NAME::Bakuhatu_End);

				int id = EffectManager::GetInstance()->PlayerAnimation(
					EffectName::eExprotion2,
					location,
					0.07f,
					false
				);
				EffectManager::GetInstance()->SetScale(id, 3.5f);

				id = EffectManager::GetInstance()->PlayerAnimation(
					EffectName::eExprotion2,
					location,
					0.07f,
					false
				);
				EffectManager::GetInstance()->SetScale(id, 2.5f);

				is_alive = false;
			}
		}

		return;
	}

	// 攻撃パターンを設定して弾を打つ
	Shot(delta_second);

	if (is_drive == true)
	{
		ripple_positions[7] = { Vector2D(-160,  100), Vector2D(160,  100) };
		ripple_positions[5] = { Vector2D(-160,  100), Vector2D(160,  100) };

	}


	damage_timer += delta_second;

	if (generate2 == true)
	{
		if (damage_timer >= 0.05f)
		{
			damage_timer = 0.0f;
			hp -= 5;
		}

	}

	// 部品を遅れて追従させる処理
	const float follow_speed = 5.0f; // 追従速度（大きいと早くついてくる）

	// 部品の相対オフセット（左右に2個ずつ）
	Vector2D offsets[6] = {
		Vector2D(110, 0), // 左奥
		Vector2D(60,  0), // 左手前
		Vector2D(-60,  0), // 右手前
		Vector2D(-110, 0),  // 右奥

		Vector2D(60, 30),  // 砲
		Vector2D(-60, 30)  // 砲
	};


	// 追従の速さ
	float individual_follow_speeds[6] = {
	12.0f,  // 左奥（ゆっくり）
	12.0f,  // 左手前
	12.0f,  // 右手前
	12.0f,   // 右奥（ゆっくり）

	10.0f,   // 砲（ゆっくり）
	10.0f   // 砲（ゆっくり）
	};

	// 追従の速さ
	float individual_follow_speeds_2[6] = {
	100.0f,  // 左奥（ゆっくり）
	100.0f,  // 左手前
	100.0f,  // 右手前
	100.0f,   // 右奥（ゆっくり）

	100.0f,   // 砲（ゆっくり）
	100.0f   // 砲（ゆっくり）
	};

	// 追従処理
	for (int i = 0; i < 6; ++i)
	{
		Vector2D target;
		if (generate == false)
		{
			target = location + offsets[i];
			part_positions[i] += (target - part_positions[i]) * individual_follow_speeds_2[i] * delta_second;
			angle = 3.14 / 1.0f;

		}
		else
		{
			target = location + offsets_2[i];
			part_positions[i] += (target - part_positions[i]) * individual_follow_speeds[i] * delta_second;
			angle = 0.0f;
		}
	}


	std::vector<int> animation_num = { 0, 1, 2, 3, 4, 5 };
	//フレームレートで時間を計測
	animation_time += delta_second;
	//8秒経ったら画像を切り替える
	if (animation_time >= anim_speed)
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
		Boss3_image[7] = Boss3_anim[animation_num[animation_count]];
	}

	if (attack_pattrn == 12)
	{
		anim_speed = 0.01;
	}
	else
	{
		anim_speed = 0.1f;
	}

	std::vector<int> jet_num = { 2, 3, 4, 8, 9, 10, 11, 12, 16, 17, 16, 12, 11, 10, 9, 8, 4, 3, 2 };
	//フレームレートで時間を計測
	jet_timer += delta_second;
	//8秒経ったら画像を切り替える
	if (jet_timer >= 0.01f)
	{
		//計測時間の初期化
		jet_timer = 0.0f;
		//時間経過カウントの増加
		jet_count++;
		//カウントがアニメーション画像の要素数以上になったら
		if (jet_count >= jet_num.size())
		{
			//カウントの初期化
			jet_count = 0;
		}
		// アニメーションが順番に代入される
		jet = Boss3_jet[jet_num[jet_count]];
	}

	if (generate) {
		show_hpbar = true;

		if (hpbar_fade_timer < 1.0f) {
			hpbar_fade_timer += delta_second;
			if (hpbar_fade_timer > 1.0f) hpbar_fade_timer = 1.0f;
		}
	}


	for (int i = 0; i < 5; ++i) {
		if (ripples[i].active) {
			ripples[i].timer += delta_second;
			if (ripples[i].timer >= 0.5f) { // 0.5秒で消滅
				ripples[i].active = false;
			}
		}
	}
	// 親クラスの更新処理を呼び出す
	__super::Update(delta_second);
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void Boss3::Draw(const Vector2D& screen_offset) const
{
	//// 雑魚１を描画する
	//DrawBox(location.x - box_size.x, location.y - box_size.y,
	//	location.x + box_size.x, location.y + box_size.y, GetColor(0, 255, 0), TRUE);

	//DrawBoss3(location);

	if (!show_hpbar) {
		DrawBoss3(location);
		return;
	}

	// ボス本体描画
	DrawBoss3(location);

	// フェード＋スケール率（0.0?1.0）
	const float t = hpbar_fade_timer;
	float scale = t;
	if (scale < 0.0f) scale = 0.0f;
	if (scale > 1.0f) scale = 1.0f;
	const int alpha = static_cast<int>(scale * 255);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	// スケーリング付き描画
	const float max_hp = 100000.0f;
	const float bar_width_full = 650.0f;
	const float bar_height = 8.0f;
	const float x_center = D_WIN_MAX_X / 2;
	const float y = 30.0f;

	// 実際の描画幅（縮小→拡大）
	const float current_bar_width = bar_width_full * scale;
	const float x = x_center - current_bar_width / 2;

	float hp_ratio = (float)hp / (float)max_hp;
	if (hp_ratio < 0.0f) hp_ratio = 0.0f;
	if (hp_ratio > 1.0f) hp_ratio = 1.0f;

	//float hp_ratio = Clamp(hp / max_hp, 0.0f, 1.0f);
	const float filled_width = current_bar_width * hp_ratio;

	// 色（例：50%以下で濃いオレンジ）
	unsigned int bar_color = (hp_ratio > 0.5f)
		? GetColor(255, 0, 0)
		: GetColor(255, 100, 0);

	// 背景
	DrawBox(x - 2, y - 2, x + current_bar_width + 2, y + bar_height + 2, GetColor(0, 0, 0), TRUE);

	// HPバー本体
	DrawBox(x, y, x + filled_width, y + bar_height, bar_color, TRUE);

	// 白枠
	DrawBox(x, y, x + current_bar_width, y + bar_height, GetColor(255, 255, 255), FALSE);

	// くぼみ（第二形態位置）
	float notch_x = x + current_bar_width / 3;
	DrawBox(notch_x - 1, y - 2, notch_x + 1, y + bar_height + 2, GetColor(255, 255, 0), TRUE);

	// HP数値
	//DrawFormatString(x + current_bar_width + 10, y - 2, GetColor(255, 255, 255), "HP: %.0f", hp);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

// 終了時処理
void Boss3::Finalize()
{
}

void Boss3::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::eShot)
	{
		if (generate2 == true)
		{
			if (on_hit == false)
			{
				// プレイヤーとの距離を取得
				float distance = (GetLocation() - player->GetLocation()).Length();

				int damage = 10;

				// 距離に応じてダメージを変更（例: 4段階）
				if (distance < 500.0f)
				{
					damage = 70;
				}
				else if (distance < 600.0f)
				{
					damage = 50;
				}
				else if (distance < 700.0f)
				{
					damage = 30;
				}
				else
				{
					damage = 30;
				}

				// 弱点ヒットなら倍率をかける
				if (is_weakness == true)
				{
					hp -= damage;
				}
				else
				{
					hp -= damage / 2;
				}

				on_hit = true;

				if (GetRand(70) == 1)
				{
					//DropItems();
				}
			}
			else
			{
				on_hit = false;
			}
		}
	}
	if (hit_object->GetCollision().object_type == eObjectType::eBeam)
	{
		if (generate2 == true)
		{
			beam_damage_timer += delta;

			if (beam_damage_timer >= 0.05f)
			{
				if (is_weakness == true)
				{
					hp -= 50;
				}
				else
				{
					hp -= 10;
				}
				beam_damage_timer = 0;

				if (GetRand(70) == 1)
				{
					//DropItems();
				}
			}
		}
	}
}

/// <summary>
/// 移動処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
void Boss3::Movement(float delta_second)
{
	move_time += delta_second;

	const float float_amplitude_y = 15.0f;
	const float float_speed_y = 2.7f;
	const float float_amplitude_x = 25.0f;
	const float float_speed_x = 2.7f;
	Vector2D float_offset;
	float_offset.y = sinf(move_time * float_speed_y) * float_amplitude_y;
	float_offset.x = cosf(move_time * float_speed_x) * float_amplitude_x;

	if (!generate)
	{
		velocity.y = -900.0f;
		base_position.x = generate_base_position.x;

		if (location.y < -400)
		{
			generate = true;
		}
	}
	else
	{
		float distance_y = base_position.y - location.y;
		float base_x = D_WIN_MAX_X / 2;
		base_position.x = generate_base_position.x - 150.0f;

		if (fabs(distance_y) > 1.0f)
		{
			const float max_speed = 200.0f;
			velocity.y = distance_y;
			if (velocity.y > max_speed) velocity.y = max_speed;
			if (velocity.y < -max_speed) velocity.y = -max_speed;

			static bool check = false;
			if (check == false)
			{
				box_size = Vector2D(200, 70);
				check = true;
			}
			else
			{
				box_size = Vector2D(100, 120);
				check = false;
			}

			collision.object_type = eObjectType::eBoss;
			collision.hit_object_type = { eObjectType::eShot, eObjectType::eBeam };

			//box_size = Vector2D(190, 80);
			image_size = 2.0f;
		}
		else
		{

			generate2 = true;

			const float swing_range = 100.0f;
			const float move_speed = 75.0f;
			// 減速距離
			const float deceleration_distance = 50.0f;
			const float min_speed = 10.0f;
			const float wait_time_max = 0.2f; // 折り返し時の停止時間

			if (!has_initialized_swing_x)
			{
				swing_center_x = base_x;
				has_initialized_swing_x = true;
				swing_direction = 1;
				swing_wait_timer = 0.0f;
			}

			float left_x = swing_center_x - swing_range;
			float right_x = swing_center_x + swing_range;

			// 停止中の場合
			if (swing_wait_timer > 0.0f)
			{
				swing_wait_timer -= delta_second;
				if (swing_wait_timer < 0.0f) swing_wait_timer = 0.0f;
				velocity.x = 0.0f;
			}
			else
			{
				float distance_to_edge = (swing_direction == 1)
					? right_x - base_position.x
					: base_position.x - left_x;

				// 端に到達
				if (distance_to_edge <= 0.0f)
				{
					swing_direction *= -1;
					swing_wait_timer = wait_time_max;
					velocity.x = 0.0f;
				}
				else
				{
					float t = Clamp(distance_to_edge / deceleration_distance, 0.0f, 1.0f);
					float eased = t * t * (3 - 2 * t); // easeInOut補間
					float current_speed = Max(move_speed * eased, min_speed);
					velocity.x = current_speed * swing_direction;
				}
			}

			velocity.y = 0.0f;
			location.y = base_position.y;
		}
	}

	generate_base_position += velocity * delta_second;
	base_position.x += velocity.x * delta_second;

	location.y = generate_base_position.y + float_offset.y;
	location.x = base_position.x + float_offset.x;
}

// 攻撃
void Boss3::Shot(float delta_second)
{
	// 次の攻撃パターン変更までの時間
	const int shot_interval = 1.0f;

	// 時間経過したら攻撃パターンを変更して弾を発射
	if (generate2 == true && is_shot == false)
	{
		auto it = ripple_positions.find(attack_pattrn);
		if (it != ripple_positions.end()) {
			const Vector2D& left_offset = it->second.first;
			const Vector2D& right_offset = it->second.second;

			for (int i = 0; i < 5; ++i) {
				if (!ripples[i].active) {
					ripples[i].active = true;
					ripples[i].timer = 0.0f;
					ripples[i].pos = location + left_offset;
					SEManager::GetInstance()->PlaySE(SE_NAME::Hamon);
					SEManager::GetInstance()->ChangeSEVolume(SE_NAME::Hamon, 60);
					break;
				}
			}

			for (int i = 0; i < 5; ++i) {
				if (!ripples[i].active) {
					ripples[i].active = true;
					ripples[i].timer = 0.0f;
					ripples[i].pos = location + right_offset;
					break;
				}
			}
		}

		shot_timer += delta_second;
		if (shot_timer >= shot_interval)
		{
#if 1

#if 0
			// 特定の攻撃のみ繰り返す
			attack_pattrn = 5;
#else
			// HPが減ったら攻撃パターンを変更（オーバーフロー防止に合わせてリセット）
			if (hp <= 3333 && attack_pattrn_num != std::vector<int>{7, 5})
			{
				is_drive = true;
				attack_pattrn_num = { 7, 5 };
				attack_count = 0; // 安全にリセット
			}

			// 決められた攻撃パターンのみ繰り返す
			if (!attack_pattrn_num.empty())
			{
				// 念のため配列サイズチェックを挟む（念押し）
				if (attack_count >= attack_pattrn_num.size())
				{
					attack_count = 0;
				}
				attack_pattrn = attack_pattrn_num[attack_count];
				attack_count++;
			}
			else
			{
				// 予備処理：攻撃パターンリストが空の時は安全なデフォルトに
				attack_pattrn = 0;
			}
#endif

#else
			// 完全ランダム
			attack_pattrn = 4 + rand() % MAX_ATTACK_PATTRN;
#endif
			is_shot = true;
			shot_timer = 0;
		}
	}

	// 攻撃
	Attack(delta_second);
}

void Boss3::DrawBoss3(const Vector2D position) const
{
	// 本体
	DrawRotaGraph(position.x, position.y, image_size, angle, Boss3_image[2], TRUE);

	// 部品
	DrawRotaGraph(part_positions[1].x, part_positions[1].y, image_size, angle, Boss3_image[3], TRUE); // 左手前
	DrawRotaGraph(part_positions[0].x, part_positions[0].y, image_size, angle, Boss3_image[5], TRUE); // 左奥
	if (is_crashing == false)
	{
		if (generate == false)
		{
			DrawRotaGraph(position.x - 70, position.y + 150, image_size, 0.0f, jet, TRUE);
			DrawRotaGraph(position.x + 70, position.y + 150, image_size, 0.0f, jet, TRUE);

		}
		else
		{
			DrawRotaGraph(position.x - 110, position.y - 250, image_size, 3.14 / 1.0f, jet, TRUE);
			DrawRotaGraph(position.x + 110, position.y - 250, image_size, 3.14 / 1.0f, jet, TRUE);
		}

	}
	DrawRotaGraph(part_positions[2].x, part_positions[2].y, image_size, angle, Boss3_image[4], TRUE); // 右手前
	DrawRotaGraph(part_positions[3].x, part_positions[3].y, image_size, angle, Boss3_image[6], TRUE); // 右奥
	DrawRotaGraph(part_positions[4].x, part_positions[4].y, image_size, angle, Boss3_image[1], TRUE);
	DrawRotaGraph(part_positions[5].x, part_positions[5].y, image_size, angle, Boss3_image[1], TRUE);


	if (is_crashing == false)
	{
		if (generate == false)
		{
			DrawRotaGraph(part_positions[4].x, part_positions[4].y - 50.0f, image_size, angle, Boss3_image[7], TRUE);
			DrawRotaGraph(part_positions[5].x, part_positions[5].y - 50.0f, image_size, angle, Boss3_image[7], TRUE);
		}
		else
		{
			DrawRotaGraph(part_positions[4].x, part_positions[4].y + 110.0f, image_size, angle, Boss3_image[7], TRUE);
			DrawRotaGraph(part_positions[5].x, part_positions[5].y + 110.0f, image_size, angle, Boss3_image[7], TRUE);
		}

	}
	//DrawBox(location.x - box_size.x, location.y - box_size.y,
	//	location.x + box_size.x, location.y + box_size.y, GetColor(0, 255, 0), TRUE);



	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); // アルファブレンドON

	if (is_crashing == false)
	{
		for (int i = 0; i < 5; ++i) {
			if (!ripples[i].active) continue;

			float t = ripples[i].timer / 0.5f; // 0.0～1.0
			if (t > 1.0f) t = 1.0f;            // 安全のためClampするならここで

			// Lerp(0.0f, 80.0f, t) → 0.0f + (80.0f - 0.0f) * t
			float radius = 80.0f * t;

			// Lerp(255.0f, 200.0f, t) → 255.0f + (200.0f - 255.0f) * t = 255 - 55 * t
			int alpha = static_cast<int>(255.0f - 55.0f * t);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

			if (is_drive == false)
			{

				DrawCircle(static_cast<int>(ripples[i].pos.x),
					static_cast<int>(ripples[i].pos.y),
					static_cast<int>(radius),
					GetColor(150, 255, 150),
					FALSE);
			}
			else
			{
				DrawCircle(static_cast<int>(ripples[i].pos.x),
					static_cast<int>(ripples[i].pos.y),
					static_cast<int>(radius),
					GetColor(255, 0, 0),
					FALSE);

			}

		}
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ブレンド無効化
}

int Boss3::GetAttackPattrn() const
{
	return attack_pattrn;
}

int Boss3::GetIsAlive() const
{
	return is_alive;
}

bool Boss3::GetGenerate() const
{
	return generate;
}

bool Boss3::GetIsCrashing() const
{
	return is_crashing;
}

// ボス２の攻撃
void Boss3::Attack(float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	if (is_shot == true)
	{
		Vector2D e_location = location;

		switch (attack_pattrn)
		{
		case 4:
			/// <summary>
			/// 攻撃パターン４（花火）
			/// </summary>
			/// <param name="bullet_num">弾の数</param>
			/// <param name="speed">弾の速度</param>
			/// <param name="spiral_interval">発射間隔</param>
			/// <param name="spiral_duration_limit">攻撃する時間</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn4_2(6, 300.0f, 1.0f, 5.0f, location, delta_second);
			break;
		case 5:
#if 0
			static float spiral_timer = 0.0f;            // 弾発射のインターバル管理
			static float spiral_angle = 0.0f;            // 回転角度
			static float spiral_total_time = 0.0f;       // パターン5に入ってからの累計時間
			const float spiral_interval = 0.1f;
			const float spiral_duration_limit = 3.0f;    // 最大発射時間

			// 3秒経過したら何もしない
			if (spiral_total_time >= spiral_duration_limit) return;

			spiral_timer += delta_second;
			spiral_total_time += delta_second;

			if (spiral_timer >= spiral_interval)
			{
				spiral_timer = 0.0f;

				float rad = spiral_angle * DX_PI / 180.0f;
				float speed = 160.0f;
				Vector2D velocity(cos(rad) * speed, sin(rad) * speed);

				GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
				EnemyShot4* shot = objm->CreateObject<EnemyShot4>(Boss3->GetLocation());
				shot->SetVelocity(velocity);

				spiral_angle += 15.0f;
				if (spiral_angle >= 360.0f) spiral_angle -= 360.0f;
			}
#else

			/// <summary>
			/// 攻撃パターン５（渦巻き）
			/// </summary>
			/// <param name="spiral_interval">発射間隔</param>
			/// <param name="spiral_duration_limit">攻撃する時間</param>
			/// <param name="spiral_speed">弾の速度</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn5_2(0.05f, 5.0f, 300.0f, location, delta_second);
			//Pattrn5(0.08f, 5.0f, 300.0f, location, delta_second);
#endif
			break;
		case 6:
			/// <summary>
			/// 攻撃パターン６（バラバラ扇型）
			/// </summary>
			/// <param name="fan_angle_range">扇の角度（60なら±30°）</param>
			/// <param name="bullet_speed">弾の速度</param>
			/// <param name="fan_interval">発射間隔</param>
			/// <param name="fan_duration_limit">攻撃する時間</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn6_2(100.0f, 300.0f, 0.15f, 10.0f, location, delta_second);

			break;
		case 7:
			/// <summary>
			/// 攻撃パターン７（段階扇形）
			/// </summary>
			/// <param name="fan_angle_range">扇の角度範囲（60なら±30°）</param>
			/// <param name="bullet_speed">弾の速度</param>
			/// <param name="fan_interval">発射の間隔</param>
			/// <param name="fan_duration_limit">攻撃時間</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn7_2(150.0f, 500.0f, 0.2f, 5.0f, Vector2D(location.x, location.y + 100.0f), delta_second);

			break;
		case 8:
			/// <summary>
			/// 攻撃パターン８（ひもQ）
			/// </summary>
			/// <param name="wave_interval">発射間隔</param>
			/// <param name="wave_duration_limit">発車時間の上限</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn8(0.1f, 1.0f, Vector2D(location.x, location.y + 150.0f), delta_second);

			break;
		case 9:
			/// <summary>
			/// 攻撃パターン９（ボス中心回転弾）
			/// </summary>
			/// <param name="shot_count">弾の数</param>
			/// <param name="radius">ボスからの距離</param>
			/// <param name="angular_speed">回転速度</param>
			/// <param name="bullet_speed">弾速</param>
			/// <param name="rotation_timer">回転の時間計測</param>
			/// <param name="generate_location">生成する位置</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn9_2(27, 250.0f, 90.0f, 0.0f, location, delta_second);

			/// <summary>
			/// 攻撃パターン11（一本ビーム）
			/// </summary>
			/// <param name="offsets_x">ボスからの距離(横)</param>
			Pattrn11(0.0f);

			break;
		case 10:
			/// <summary>
			/// 攻撃パターン１０（ベクトル中心回転弾）
			/// </summary>
			/// <param name="shot_count">弾の数</param>
			/// <param name="radius">ボスからの距離</param>
			/// <param name="angular_speed">回転速度</param>
			/// <param name="bullet_speed">弾速</param>
			/// <param name="rotation_timer">回転の時間計測</param>
			/// <param name="center_location">生成する場所</param>
			/// <param name="generate_location">生成する場所</param>
			/// <param name="delta_second">１フレームあたりの時間（基本的に変更なし）</param>
			Pattrn10_2(8, 120.0f, 550.0f, 450.0f, 5.0f, location, delta_second);

			break;
		case 11:
			/// <summary>
			/// 攻撃パターン11（一本ビーム）
			/// </summary>
			/// <param name="offsets_x">ボスからの距離(横)</param>
			Pattrn11(0.0f);

			break;
		case 12:
			/// <summary>
			/// 攻撃パターン12（二本ビーム）
			/// </summary>
			Pattrn12();

			break;
		case 13:
			Pattrn13(delta_second);

			break;
		default:
			break;
		}
	}
}

/// <summary>
/// 攻撃パターン４
/// </summary>
/// <param name="bullet_num">弾の数</param>
/// <param name="speed">弾の速度</param>
/// <param name="spiral_interval">発射間隔</param>
/// <param name="spiral_duration_limit">攻撃する時間</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn4(int bullet_num, float speed, float spiral_interval, float spiral_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float spiral_timer = 0.0f;
	static float spiral_total_time = 0.0f;

	spiral_timer += delta_second;
	spiral_total_time += delta_second;

	if (spiral_timer >= spiral_interval)
	{
		spiral_timer = 0.0f;

		for (int i = 0; i < bullet_num; i++)
		{
			float angle = (360.0f / bullet_num) * i;
			float rad = angle * DX_PI / 180.0f;

			Vector2D velocity(cos(rad) * speed, sin(rad) * speed);

			EnemyBullet3* e_shot4 = objm->CreateObject<EnemyBullet3>(generate_location);
			e_shot4->SetVelocity(velocity);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

		}
	}

	// 時間制限を超えたら終了（発射しない）
	if (spiral_total_time >= spiral_duration_limit)
	{
		spiral_total_time = 0.0f;
		is_shot = false;
	}
}

void Boss3::Pattrn4_2(int bullet_num, float speed, float spiral_interval, float spiral_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float spiral_timer = 0.0f;
	static float spiral_total_time = 0.0f;

	spiral_timer += delta_second;
	spiral_total_time += delta_second;

	if (spiral_timer >= spiral_interval)
	{
		spiral_timer = 0.0f;

		for (int i = 0; i < bullet_num; i++)
		{
			float angle = (360.0f / bullet_num) * i;
			float rad = angle * DX_PI / 180.0f;

			Vector2D velocity(cos(rad) * speed, sin(rad) * speed);

			EnemyBullet3* e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x - 170.0f, generate_location.y + 65.0f));
			e_shot4->SetVelocity(velocity);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);


		}

		for (int i = 0; i < bullet_num; i++)
		{
			float angle = (360.0f / bullet_num) * i;
			float rad = angle * DX_PI / 180.0f;

			Vector2D velocity(cos(rad) * speed, sin(rad) * speed);

			EnemyBullet3* e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x + 170.0f, generate_location.y + 65.0f));
			e_shot4->SetVelocity(velocity);
		}
	}

	// 時間制限を超えたら終了（発射しない）
	if (spiral_total_time >= spiral_duration_limit)
	{
		spiral_total_time = 0.0f;
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン５
/// </summary>
/// <param name="spiral_interval">発射間隔</param>
/// <param name="spiral_duration_limit">攻撃する時間</param>
/// <param name="spiral_speed">弾の速度</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn5(float spiral_interval, float spiral_duration_limit, float spiral_speed, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float spiral_timer = 0.0f;
	static float spiral_angle = 0.0f;
	static float spiral_total_time = 0.0f;

	spiral_timer += delta_second;
	spiral_total_time += delta_second;

	if (spiral_timer >= spiral_interval)
	{
		spiral_timer = 0.0f;

#if 0
		// 上下二方向に出す
		for (int dir = 0; dir < 2; dir++)
		{
			float base_angle = (dir == 0) ? 90.0f : 270.0f;
			float current_angle = base_angle + spiral_angle;

			float rad = current_angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

			EnemyShot4* shot = objm->CreateObject<EnemyShot4>(generate_location);
			shot->SetVelocity(velocity);
			shot->SetAttackPattrn(2);
		}
#else
		// 一方向
		float base_angle = 90.0f; // 上方向
		float current_angle = base_angle + spiral_angle;

		float rad = current_angle * DX_PI / 180.0f;
		Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

		EnemyBullet3* shot = objm->CreateObject<EnemyBullet3>(generate_location);
		shot->SetVelocity(velocity);
		SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
		SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

#endif


		spiral_angle += 40.0f;
		if (spiral_angle >= 360.0f) spiral_angle -= 360.0f;
	}

	// 時間制限を超えたら終了（発射しない）
	if (spiral_total_time >= spiral_duration_limit)
	{
		spiral_total_time = 0.0f;
		is_shot = false;
	}
}

void Boss3::Pattrn5_2(float spiral_interval, float spiral_duration_limit, float spiral_speed, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float spiral_timer = 0.0f;
	static float spiral_angle = 0.0f;
	static float spiral_total_time = 0.0f;

	spiral_timer += delta_second;
	spiral_total_time += delta_second;

	if (spiral_timer >= spiral_interval)
	{
		spiral_timer = 0.0f;

#if 0
		for (int dir = 0; dir < 2; dir++)
		{
			float base_angle = (dir == 0) ? 90.0f : 270.0f;
			float current_angle = base_angle + spiral_angle;

			float rad = current_angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

			EnemyShot4* shot = objm->CreateObject<EnemyShot4>(Vector2D(generate_location.x + 170.0f, generate_location.y + 65.0f));
			shot->SetVelocity(velocity);
			shot->SetAttackPattrn(2);
		}

		for (int dir = 0; dir < 2; dir++)
		{
			float base_angle = (dir == 0) ? 90.0f : 270.0f;
			float current_angle = base_angle - spiral_angle; // ← 逆回転にする

			float rad = current_angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

			EnemyShot4* shot = objm->CreateObject<EnemyShot4>(Vector2D(generate_location.x - 170.0f, generate_location.y + 65.0f));
			shot->SetVelocity(velocity);
			shot->SetAttackPattrn(2);
		}
#else

		// 右側（時計回り、上方向のみ）
		{
			float base_angle = 90.0f;
			// マイナスにしたら回転が反転する
			float current_angle = base_angle + spiral_angle;

			float rad = current_angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

			EnemyBullet3* shot = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x + 170.0f, generate_location.y + 65.0f));
			shot->SetVelocity(velocity);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

		}

		// 左側（反時計回り、上方向のみ）
		{
			float base_angle = 90.0f;
			float current_angle = base_angle + spiral_angle;

			float rad = current_angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * spiral_speed, sin(rad) * spiral_speed);

			EnemyBullet3* shot = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x - 170.0f, generate_location.y + 65.0f));
			shot->SetVelocity(velocity);
		}


#endif


		spiral_angle += 40.0f;
		if (spiral_angle >= 360.0f) spiral_angle -= 360.0f;
	}

	// 時間制限を超えたら終了（発射しない）
	if (spiral_total_time >= spiral_duration_limit)
	{
		spiral_total_time = 0.0f;
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン６
/// </summary>
/// <param name="fan_angle_range">扇の角度（60なら±30°）</param>
/// <param name="bullet_speed">弾の速度</param>
/// <param name="fan_interval">発射間隔</param>
/// <param name="fan_duration_limit">攻撃する時間</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn6(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	// 攻撃持続用 static 変数
	static float fan_timer = 0.0f;
	static float fan_total_time = 0.0f;

	// 攻撃パターンが変更されたらリセットしたい → Boss3側で状態持たせるのもあり

	fan_timer += delta_second;
	fan_total_time += delta_second;

	// 一定間隔ごとに発射
	if (fan_timer >= fan_interval)
	{
		fan_timer = 0.0f;

		// ランダムな角度（中心±60°）
		float base_angle = 90.0f; // 下方向中心
		float random_angle = base_angle - fan_angle_range / 2.0f + (rand() % (int)fan_angle_range);

		float rad = random_angle * DX_PI / 180.0f;
		Vector2D velocity(cos(rad) * bullet_speed, sin(rad) * bullet_speed);

		e_shot4 = objm->CreateObject<EnemyBullet3>(generate_location);
		e_shot4->SetVelocity(velocity);
		SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
		SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);
	}

	// 時間制限を超えたら終了（発射しない）
	if (fan_total_time >= fan_duration_limit)
	{
		fan_total_time = 0.0f;
		is_shot = false;
	}
}

void Boss3::Pattrn6_2(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	// 攻撃持続用 static 変数
	static float fan_timer = 0.0f;
	static float fan_total_time = 0.0f;

	// 攻撃パターンが変更されたらリセットしたい → Boss3側で状態持たせるのもあり

	fan_timer += delta_second;
	fan_total_time += delta_second;

	// 一定間隔ごとに発射
	if (fan_timer >= fan_interval)
	{
		fan_timer = 0.0f;

		// ランダムな角度（中心±60°）
		float base_angle = 90.0f; // 下方向中心
		float random_angle = base_angle - fan_angle_range / 2.0f + (rand() % (int)fan_angle_range);

		float rad = random_angle * DX_PI / 180.0f;
		Vector2D velocity(cos(rad) * bullet_speed, sin(rad) * bullet_speed);

		e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x + 170.0f, generate_location.y + 65.0f));
		e_shot4->SetVelocity(velocity);
		e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x - 170.0f, generate_location.y + 65.0f));
		e_shot4->SetVelocity(velocity);
		SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
		SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

	}

	// 時間制限を超えたら終了（発射しない）
	if (fan_total_time >= fan_duration_limit)
	{
		fan_total_time = 0.0f;
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン７
/// </summary>
/// <param name="fan_angle_range">扇の角度範囲（60なら±30°）</param>
/// <param name="bullet_speed">弾の速度</param>
/// <param name="fan_interval">発射の間隔</param>
/// <param name="fan_duration_limit">攻撃時間</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn7(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float fan_timer = 0.0f;
	static float fan_total_time = 0.0f;

	fan_timer += delta_second;
	fan_total_time += delta_second;

	if (fan_timer >= fan_interval)
	{
		fan_timer = 0.0f;

		int bullet_count = 4;
		float base_angle = 90.0f; // 中心下方向

		for (int i = 0; i < bullet_count; ++i)
		{
			float angle = base_angle - fan_angle_range / 2.0f +
				(fan_angle_range / (bullet_count - 1)) * i;

			float rad = angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * bullet_speed, sin(rad) * bullet_speed);

			e_shot4 = objm->CreateObject<EnemyBullet3>(generate_location);
			e_shot4->SetVelocity(velocity);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

		}
	}

	// 一定時間経過したら終了
	if (fan_total_time >= fan_duration_limit)
	{
		fan_total_time = 0.0f;
		is_shot = false;
	}
}

void Boss3::Pattrn7_2(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float fan_timer = 0.0f;
	static float fan_total_time = 0.0f;

	fan_timer += delta_second;
	fan_total_time += delta_second;

	if (fan_timer >= fan_interval)
	{
		fan_timer = 0.0f;

		int bullet_count = 6;
		float base_angle = 90.0f; // 中心下方向

		for (int i = 0; i < bullet_count; ++i)
		{
			float angle = base_angle - fan_angle_range / 2.0f +
				(fan_angle_range / (bullet_count - 1)) * i;

			float rad = angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * bullet_speed, sin(rad) * bullet_speed);

			e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x + 170.0f, generate_location.y - 10.0f));
			e_shot4->SetVelocity(velocity);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

		}

		for (int i = 0; i < bullet_count; ++i)
		{
			float angle = base_angle - fan_angle_range / 2.0f +
				(fan_angle_range / (bullet_count - 1)) * i;

			float rad = angle * DX_PI / 180.0f;
			Vector2D velocity(cos(rad) * bullet_speed, sin(rad) * bullet_speed);

			e_shot4 = objm->CreateObject<EnemyBullet3>(Vector2D(generate_location.x - 170.0f, generate_location.y - 10.0f));
			e_shot4->SetVelocity(velocity);
		}
	}

	// 一定時間経過したら終了
	if (fan_total_time >= fan_duration_limit)
	{
		fan_total_time = 0.0f;
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン８
/// </summary>
/// <param name="wave_interval">発射間隔</param>
/// <param name="wave_duration_limit">発車時間の上限</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn8(float wave_interval, float wave_duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

#if 1
	static float wave_timer = 0.0f;
	static float wave_total_time = 0.0f;

	wave_timer += delta_second;
	wave_total_time += delta_second;

	if (wave_timer >= wave_interval)
	{
		wave_timer = 0.0f;
		Vector2D e_lo = generate_location;

		// 右側
		e_shot5 = objm->CreateObject<EnemyBullet4>(Vector2D(e_lo.x + 50, e_lo.y));
		e_shot5->SetWaveReflected(false);
		e_shot5->SetVelocity(Vector2D(0, 200));
		e_shot5->SetWaveParameters(600.0f, 0.7f);

		// 左側
		e_shot5 = objm->CreateObject<EnemyBullet4>(Vector2D(e_lo.x - 50, e_lo.y));
		e_shot5->SetWaveReflected(true);
		e_shot5->SetVelocity(Vector2D(0, 200));
		e_shot5->SetWaveParameters(600.0f, 0.7f);
		SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
		SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

	}

	// 一定時間経過したら終了
	if (wave_total_time >= wave_duration_limit)
	{
		wave_total_time = 0.0f;
		is_shot = false;
	}
#else

	int num_shots = 20;
	float spread_speed = 150.0f;
	Vector2D origin = e_location; // 弾の初期発射位置（ボスの位置）
	Vector2D boss_center = Boss3->GetLocation(); // 吸い込み中心！

	for (int i = 0; i < num_shots; ++i)
	{
		float angle = 360.0f / num_shots * i;
		float rad = angle * DX_PI / 180.0f;

		e_shot5 = objm->CreateObject<EnemyShot5>(origin);
		e_shot5->SetVelocity(Vector2D(cosf(rad), sinf(rad)) * spread_speed);
		e_shot5->SetSuckCenter(boss_center); // 吸い込み先をセット！
	}
#endif

}

/// <summary>
/// 攻撃パターン９
/// </summary>
/// <param name="shot_count">弾の数</param>
/// <param name="radius">ボスからの距離</param>
/// <param name="angular_speed">回転速度</param>
/// <param name="bullet_speed">弾速</param>
/// <param name="rotation_timer">回転の時間計測</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn9(int shot_count, float radius, float angular_speed, float bullet_speed, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static std::vector<float> angles;    // 各弾の角度管理
	static std::vector<EnemyBullet3*> rotating_shots;  // 弾のポインタを保持

	static float rotation_timer = 0.0f;

	rotation_timer += delta_second;

	// 初期化処理
	if (angles.empty())
	{
		rotating_shots.clear();
		// 弾を生成して初期角度を設定
		for (int i = 0; i < shot_count; ++i)
		{
			float angle = 360.0f / shot_count * i;  // 弾の角度設定
			angles.push_back(angle);

			// ボスの周囲に弾を配置
			e_shot4 = objm->CreateObject<EnemyBullet3>(generate_location);
			SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
			SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

			if (e_shot4)
			{
				rotating_shots.push_back(e_shot4);  // 正しく生成されたら弾を保存
				e_shot4->SetVelocity(Vector2D(0, 0)); // 回転だけなので弾速は0

			}
		}
	}

	// 各弾の位置を更新して回転させる
	for (int i = 0; i < shot_count; ++i)
	{
		angles[i] += angular_speed * delta_second; // 角度更新
		if (angles[i] >= 360.0f) angles[i] -= 360.0f; // 角度を0?360度に調整

		float rad = angles[i] * DX_PI / 180.0f;  // ラジアンに変換

		// ボスを中心に半径 `radius` の円軌道を描く
		Vector2D new_pos = generate_location + Vector2D(cos(rad) * radius, sin(rad) * radius);

		// 弾の位置を更新
		if (rotating_shots[i])  // 弾が正しく存在するか確認
		{
			rotating_shots[i]->SetLocation(new_pos);

			// 弾の速度はゼロ（回転のみ）
			rotating_shots[i]->SetVelocity(Vector2D(0, 0));
		}
	}

	// 攻撃の終了判定（一定時間経過後）
	static float duration_timer = 0.0f;
	duration_timer += delta_second;
	if (duration_timer >= 5.0f)  // 5秒後に終了
	{
		for (int i = 0; i < shot_count; ++i)
		{
			if (rotating_shots[i])
			{
				rotating_shots[i]->SetDestroy();  // 弾を削除
			}

		}
		angles.clear();  // 角度情報もクリア
		rotating_shots.clear();  // 弾のリストもクリア
		duration_timer = 0.0f;
		is_shot = false;
	}
}

void Boss3::Pattrn9_2(int shot_count, float radius, float angular_speed, float bullet_speed, const Vector2D& generate_location, float delta_second)
{
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static std::vector<float> angles_left;
	static std::vector<float> angles_right;
	static std::vector<EnemyBullet3*> shots_left;
	static std::vector<EnemyBullet3*> shots_right;

	static float rotation_timer = 0.0f;
	rotation_timer += delta_second;

	const float offset_x = 180.0f; // ボスの中心から左右にずらす距離
	Vector2D left_center = generate_location + Vector2D(-offset_x, 0);
	Vector2D right_center = generate_location + Vector2D(+offset_x, 0);

	// 初期化
	if (angles_left.empty() && angles_right.empty())
	{
		shots_left.clear();
		shots_right.clear();

		for (int i = 0; i < shot_count; ++i)
		{
			float angle = 360.0f / shot_count * i;
			EnemyBullet3* e_shot4 = nullptr;

			if (i % 2 == 0)
			{
				angles_left.push_back(angle);
				e_shot4 = objm->CreateObject<EnemyBullet3>(left_center);
				SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
				SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

				if (e_shot4)
				{
					shots_left.push_back(e_shot4);
					e_shot4->SetVelocity(Vector2D(0, 0));
					//e_shot4->SetAttackPattrn(1);
				}
			}
			else
			{
				angles_right.push_back(angle);
				e_shot4 = objm->CreateObject<EnemyBullet3>(right_center);
				if (e_shot4)
				{
					shots_right.push_back(e_shot4);
					e_shot4->SetVelocity(Vector2D(0, 0));
					//e_shot4->SetAttackPattrn(1);
				}
			}
		}
	}

	// 左側の弾を更新
	for (size_t i = 0; i < angles_left.size(); ++i)
	{
		angles_left[i] += angular_speed * delta_second;
		if (angles_left[i] >= 360.0f) angles_left[i] -= 360.0f;

		float rad = angles_left[i] * DX_PI / 180.0f;
		Vector2D new_pos = left_center + Vector2D(cos(rad) * radius, sin(rad) * radius);

		if (i < shots_left.size() && shots_left[i])
		{
			shots_left[i]->SetLocation(new_pos);
			shots_left[i]->SetVelocity(Vector2D(0, 0));
		}
	}

	// 右側の弾を更新
	for (size_t i = 0; i < angles_right.size(); ++i)
	{
		angles_right[i] += angular_speed * delta_second;
		if (angles_right[i] >= 360.0f) angles_right[i] -= 360.0f;

		float rad = angles_right[i] * DX_PI / 180.0f;
		Vector2D new_pos = right_center + Vector2D(cos(rad) * radius, sin(rad) * radius);

		if (i < shots_right.size() && shots_right[i])
		{
			shots_right[i]->SetLocation(new_pos);
			shots_right[i]->SetVelocity(Vector2D(0, 0));
		}
	}

	// 攻撃終了処理（5秒経過で削除）
	static float duration_timer = 0.0f;
	duration_timer += delta_second;
	if (duration_timer >= 5.0f)
	{
		for (auto& s : shots_left)
			if (s) s->SetDestroy();
		for (auto& s : shots_right)
			if (s) s->SetDestroy();

		angles_left.clear();
		angles_right.clear();
		shots_left.clear();
		shots_right.clear();
		duration_timer = 0.0f;
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン１０
/// </summary>
/// <param name="shot_count">弾の数</param>
/// <param name="radius">ボスからの距離</param>
/// <param name="angular_speed">回転速度</param>
/// <param name="bullet_speed">弾速</param>
/// <param name="rotation_timer">回転の時間計測</param>
/// <param name="generate_location">生成する場所</param>
/// <param name="delta_second">１フレームあたりの時間</param>
void Boss3::Pattrn10(int shot_count, float radius, float angular_speed, float center_speed, float duration_limit, const Vector2D& generate_location, float delta_second)
{
	// オブジェクト管理クラスのインスタンスを取得
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float duration_timer = 0.0f;
	static Vector2D center_pos;  // 見えない中心座標
	static Vector2D center_velocity;
	static std::vector<float> angles;
	static std::vector<EnemyBullet3*> rotating_shots;
	static bool initialized = false;

	// 時間経過
	duration_timer += delta_second;

	if (duration_timer < duration_limit)
	{
		if (!initialized) // 初期化処理
		{
			// ボスの位置を初期中心として、真下に移動させる
			center_pos = generate_location;
			center_velocity = Vector2D(0.0f, center_speed); // 真下方向

			angles.clear();
			rotating_shots.clear();
			for (int i = 0; i < shot_count; ++i)
			{
				float angle = 360.0f / shot_count * i;
				angles.push_back(angle);

				EnemyBullet3* shot = objm->CreateObject<EnemyBullet3>(center_pos);
				SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
				SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

				if (shot)
				{
					shot->SetVelocity(Vector2D(0, 0));
					rotating_shots.push_back(shot);
				}
			}

			initialized = true;
		}

		// 見えない中心の移動
		center_pos += center_velocity * delta_second;

		// 回転弾の位置更新
		for (int i = 0; i < shot_count; ++i)
		{
			angles[i] += angular_speed * delta_second;
			if (angles[i] >= 360.0f) angles[i] -= 360.0f;

			float rad = angles[i] * DX_PI / 180.0f;
			Vector2D offset(cos(rad) * radius, sin(rad) * radius);

			if (rotating_shots[i])
			{
				rotating_shots[i]->SetLocation(center_pos + offset);
			}
		}
	}
	else
	{
		// 攻撃終了後の処理
		duration_timer = 0.0f;
		is_shot = false;
		initialized = false;  // 初期化フラグをリセット
	}
}

void Boss3::Pattrn10_2(int shot_count, float radius, float angular_speed, float center_speed, float duration_limit, const Vector2D& center_location, float delta_second)
{
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	static float duration_timer = 0.0f;
	static Vector2D center_pos_L, center_pos_R;
	static Vector2D center_velocity;
	static std::vector<RotatingShotData> shot_data_L, shot_data_R;
	static bool initialized = false;
	static float arrival_phase_timer = 0.0f;
	static bool all_arrived = false;


	duration_timer += delta_second;

	if (duration_timer < duration_limit)
	{
		if (!initialized)
		{
			const float offset_x = 170.0f;

			center_pos_L = center_location + Vector2D(-offset_x, 50);
			center_pos_R = center_location + Vector2D(+offset_x, 50);
			center_velocity = Vector2D(0.0f, center_speed);

			shot_data_L.clear();
			shot_data_R.clear();

			for (int i = 0; i < shot_count; ++i)
			{
				float angle = 360.0f / shot_count * i;
				float rad = angle * DX_PI / 180.0f;
				Vector2D offset(cos(rad) * radius, sin(rad) * radius);

				// 左側の弾
				EnemyBullet3* shot_L = objm->CreateObject<EnemyBullet3>(center_pos_L);
				SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
				SEManager::GetInstance()->ChangeSEVolume(SE_NAME::EnemyShot, 50);

				if (shot_L)
				{
					shot_L->SetVelocity(Vector2D(0, 0));
					shot_data_L.push_back({ shot_L, angle, offset, 0.0f, RotatingShotData::State::MoveToCircle });
				}

				// 右側の弾
				EnemyBullet3* shot_R = objm->CreateObject<EnemyBullet3>(center_pos_R);
				if (shot_R)
				{
					shot_R->SetVelocity(Vector2D(0, 0));
					shot_data_R.push_back({ shot_R, angle, offset, 0.0f, RotatingShotData::State::MoveToCircle });
				}
			}

			initialized = true;
		}

		// 中心移動
		if (all_arrived)
		{
			center_pos_L += center_velocity * delta_second;
			center_pos_R += center_velocity * delta_second;
		}

		auto update_shots = [&](std::vector<RotatingShotData>& shot_data, const Vector2D& center_pos)
			{
				for (auto& data : shot_data)
				{
					if (!data.shot) continue;

					switch (data.state)
					{
					case RotatingShotData::State::MoveToCircle:
					{
						data.arrival_timer += delta_second;
						float t = Min(data.arrival_timer / 1.0f, 1.0f);
						float eased = 1.0f - pow(1.0f - t, 2.0f); // easeOut

						Vector2D pos = center_pos + data.target_offset * eased;
						data.shot->SetLocation(pos);

						if (t >= 1.0f)
							data.state = RotatingShotData::State::Rotate;

						bool all_ready = true;
						for (auto& d : shot_data_L) if (d.state != RotatingShotData::State::Rotate) all_ready = false;
						for (auto& d : shot_data_R) if (d.state != RotatingShotData::State::Rotate) all_ready = false;

						if (all_ready) all_arrived = true;
						break;
					}
					case RotatingShotData::State::Rotate:
					{
						data.angle_deg += angular_speed * delta_second;
						if (data.angle_deg >= 360.0f) data.angle_deg -= 360.0f;

						float rad = data.angle_deg * DX_PI / 180.0f;
						Vector2D offset(cos(rad) * radius, sin(rad) * radius);
						data.shot->SetLocation(center_pos + offset);
						break;
					}
					}
				}
			};

		// 左右の弾更新
		update_shots(shot_data_L, center_pos_L);
		update_shots(shot_data_R, center_pos_R);
	}
	else
	{
		// 攻撃終了処理
		for (auto& d : shot_data_L) if (d.shot) d.shot->SetDestroy();
		for (auto& d : shot_data_R) if (d.shot) d.shot->SetDestroy();

		shot_data_L.clear();
		shot_data_R.clear();
		initialized = false;
		all_arrived = false;              // ← これが大事
		duration_timer = 0.0f;
		arrival_phase_timer = 0.0f;       // ← 使っているなら
		is_shot = false;
	}
}

/// <summary>
/// 攻撃パターン11（一本ビーム）
/// </summary>
/// <param name="offsets_x">ボスからの距離</param>
void Boss3::Pattrn11(float offsets_x)
{
	static bool                         beam_on = false;          // いま発射中か？
	static EnemyBeam* b;                     // 発射中ビーム

	if (!beam_on)
	{
		GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

		b = objm->CreateObject<EnemyBeam>(
			Vector2D(
				location.x + offsets_x,
				(location.y + 150.0f) - box_size.y
			));
		b->SetBoss3(this);
		beam_on = true;
	}

	if (b != nullptr)
	{
		// それぞれ +100 / -100 のオフセットで追従
		b->SetLocation(
			Vector2D(
				location.x + offsets_x,
				(location.y + 150.0f) + b->GetBoxSize().y
			));
	}

	if (b != nullptr && b->is_destroy)   // 5 秒後などに true になる想定
	{
		b->SetDestroy();                 // 必要なら明示削除
		is_shot = false;
		beam_on = false;
	}
}

/// <summary>
/// 攻撃パターン12（ビーム二本）
/// </summary>
void Boss3::Pattrn12()
{
	static bool                         beam_on = false;          // いま発射中か？
	static std::vector<EnemyBeam*>     beams;                     // 発射中ビーム
	static const float                  OFFSETS_X[2] = { +100.f, -100.f }; // +100 / -100 の並び

	if (!beam_on)
	{
		PlaySoundMem(se_beam, DX_PLAYTYPE_BACK);
		GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

		for (int i = 0; i < 2; ++i)
		{
			EnemyBeam* b = objm->CreateObject<EnemyBeam>(
				Vector2D(
					location.x + OFFSETS_X[i],
					(location.y + 50.0f) - box_size.y
				));
			b->SetBoss3(this);
			beams.push_back(b);
		}
		beam_on = true;
		SEManager::GetInstance()->PlaySE(SE_NAME::EnemyBeam);
	}

	for (size_t i = 0; i < beams.size(); ++i)
	{
		if (beams[i] == nullptr) continue;   // 念のため

		// それぞれ +100 / -100 のオフセットで追従
		beams[i]->SetLocation(
			Vector2D(
				location.x + OFFSETS_X[i],
				(location.y + 50.0f) + beams[i]->GetBoxSize().y
			));
	}

	for (auto it = beams.begin(); it != beams.end(); )
	{
		EnemyBeam* b = *it;

		if (b != nullptr && b->is_destroy)   // 5 秒後などに true になる想定
		{
			b->SetDestroy();                 // 必要なら明示削除
			it = beams.erase(it);            // vector から外す
		}
		else
		{
			++it;
		}
	}

	// 全部消えたら次の攻撃を解禁
	if (beam_on && beams.empty())
	{
		is_shot = false;
		beam_on = false;
	}
}

/// <summary>
/// 攻撃パターン13（ビームの段階攻撃）バグあり
/// </summary>
/// <param name="delta_second">1フレーム当たりの時間</param>
void Boss3::Pattrn13(float delta_second)
{
	static int   step = 0;
	static float timer = 0.0f;
	static std::vector<EnemyBeam*> beams;

	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

	timer += delta_second; // 可変FPSなら delta_second を使う

	switch (step)
	{
	case 0: // 第1段階：中央に1本
	{
		EnemyBeam* b = objm->CreateObject<EnemyBeam>(
			Vector2D(location.x, location.y - D_OBJECT_SIZE)
		);
		b->SetBoss3(this);
		beams.push_back(b);
		step = 1;
		timer = 0.0f;
	}
	break;

	case 1: // 2秒後に第2段階：2本（左右）
		if (timer >= 2.0f)
		{
			float offsets[] = { +100.0f, -100.0f };
			for (float ox : offsets)
			{
				EnemyBeam* b = objm->CreateObject<EnemyBeam>(
					Vector2D(location.x + ox, location.y - D_OBJECT_SIZE)
				);
				b->SetBoss3(this);
				beams.push_back(b);
			}
			step = 2;
			timer = 0.0f;
		}
		break;

	case 2: // 2秒後に第3段階：3本（全体）
		if (timer >= 2.0f)
		{
			float offsets[] = { +200.0f, 0.0f, -200.0f };
			for (float ox : offsets)
			{
				EnemyBeam* b = objm->CreateObject<EnemyBeam>(
					Vector2D(location.x + ox, location.y - D_OBJECT_SIZE)
				);
				b->SetBoss3(this);
				beams.push_back(b);
			}
			step = 3;
		}
		break;

	case 3: // ビーム位置追従と終了判定
		for (size_t i = 0; i < beams.size(); ++i)
		{
			if (beams[i] == nullptr) continue;

			float offset = 0.0f;
			switch (beams.size())
			{
			case 1: offset = 0.0f; break;
			case 3: offset = (i == 0) ? +100.0f : (i == 1) ? -100.0f : 0.0f; break;
			case 6: offset = (i == 0) ? +0.0f : (i == 1) ? +100.0f : (i == 2) ? -100.0f :
				(i == 3) ? +200.0f : (i == 4) ? 0.0f : -200.0f;
				break;
			}

			beams[i]->SetLocation(
				Vector2D(
					location.x + offset,
					(location.y - D_OBJECT_SIZE) + beams[i]->GetBoxSize().y
				)
			);
		}

		// ビームがすべて消えたら終了
		for (auto it = beams.begin(); it != beams.end(); )
		{
			EnemyBeam* b = *it;
			if (b != nullptr && b->is_destroy)
			{
				b->SetDestroy();
				it = beams.erase(it);
			}
			else ++it;
		}

		if (beams.empty())
		{
			is_shot = false;
			step = 0;
			timer = 0.0f;
		}
		break;
	}
}