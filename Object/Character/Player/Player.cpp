#include "Player.h"
#include "../../../Utility/InputManager.h"
#include "../../../Utility/ProjectConfig.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../Bullet/PlayerBullet/PlayerAttackBullet.h"
#include "../../Bullet/PlayerBullet/PlayerDefenceBullet.h"
#include "../../Beam/PlayerBeam.h"

Player::Player() : is_shot(false), life(1), on_hit(false), is_damage(false)
{
}

Player::~Player()
{
}

// 初期化処理
void Player::Initialize()
{
	z_layer = 5;
	velocity = 0;
	box_size = 10;


	// 当たり判定のオブジェクト設定
	collision.is_blocking = true;
	// 自分のオブジェクトタイプ
	collision.object_type = eObjectType::ePlayer;
	// 当たる相手のオブジェクトタイプ
	collision.hit_object_type.push_back(eObjectType::eEnemy);
	collision.hit_object_type.push_back(eObjectType::eBoss);
	collision.hit_object_type.push_back(eObjectType::eExp);
	collision.hit_object_type.push_back(eObjectType::ePowerUp);

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	attack = rm->GetImages("Resource/Image/Object/Player/color04/player03.png")[0];
	defence = rm->GetImages("Resource/Image/Object/Player/color02/player03.png")[0];
	image = attack;

	// プレイヤーの傾き画像
	attack_player_image_left = rm->GetImages("Resource/Image/Object/Player/color04/anime_player03_L01.png", 2, 2, 1, 56, 64);
	attack_player_image_right = rm->GetImages("Resource/Image/Object/Player/color04/anime_player03_R01.png", 2, 2, 1, 56, 64);
	defence_player_image_left = rm->GetImages("Resource/Image/Object/Player/color02/anime_player03_L01.png", 2, 2, 1, 56, 64);
	defence_player_image_right = rm->GetImages("Resource/Image/Object/Player/color02/anime_player03_R01.png", 2, 2, 1, 56, 64);
	player_image_left = attack_player_image_left;
	player_image_right = attack_player_image_right;

	// プレイヤーのジェット部分の画像
	attack_player_jet = rm->GetImages("Resource/Image/Object/Player/Shot/anime_effect17.png", 6, 6, 1, 8, 88);
	defence_player_jet = rm->GetImages("Resource/Image/Object/Player/Shot/anime_effect16.png", 6, 6, 1, 8, 88);
	player_jet = attack_player_jet;
	jet = player_jet[2];

	// プレイヤーのノズル部分の画像
	nozzles = rm->GetImages("Resource/Image/Effect/293.png", 72, 8, 9, 64, 64);
	nozzle = nozzles[17];
	attack_nozzles = { 17, 18, 19, 20, 21, 22, 23, 24, 25 };
	defence_nozzles = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	nozzle_type = attack_nozzles;

	shields2 = rm->GetImages("Resource/Image/Object/Item/Shield/pipo-btleffect206_480.png", 20, 5, 4, 480, 480);
	shields = rm->GetImages("Resource/Image/Object/Item/Shield/pipo-btleffect206h_480.png", 15, 5, 3, 480, 480);

	shot_interval = 0.07f;
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void Player::Update(float delta_second)
{
	// 移動処理
	Movement(delta_second);
	// ダメージを受けたら
	Damage(delta_second);

	if (is_dead_animation_playing)
	{
		dead_animation_timer += delta_second;

		if (dead_animation_timer >= dead_animation_duration)
		{
			if (is_alive)
			{
				EffectManager* fm = Singleton<EffectManager>::GetInstance();
				int id = fm->PlayerAnimation(EffectName::eExprotion2, location, 0.05f, false); // スケール2倍

				is_alive = false;
			}
		}
		else
		{
			game_over_player = true;
			shot_stop = true;

			//// 爆発演出を定期的に再生
			//if (dead_animation_timer - last_explosion_time > 0.2f)  // 0.2秒ごとに
			//{
			//	last_explosion_time = dead_animation_timer;

			//	Vector2D effect_pos = location;
			//	effect_pos.x += GetRand(30);
			//	effect_pos.y += GetRand(30);

			//	EffectManager* fm = Singleton<EffectManager>::GetInstance();
			//	int id = fm->PlayerAnimation(EffectName::eExprotion2, effect_pos, 0.05f, false); // スケール2倍
			//	fm->SetScale(id, 1.0f);
			//}
		}
	}
	else
	{
		// 打てる状態だったら打つ
		if (shot_stop == false)
		{
			// 弾を打つ
			Shot(delta_second);
		}

		// ライフが０になったらゲームオーバー
		if (life < 0)
		{
			is_dead_animation_playing = true;
			dead_animation_timer = 0.0f;
		}

		if (powerd_time > 0.0f)
		{
			powerd_time -= delta_second;
		}
		else
		{
			powerd_on = false;
		}

		if (powerd >= 3)
		{
			powerd = 3;
		}
		else if (powerd <= 1)
		{
			powerd = 1;
		}

		// 部品のアニメーション
		BuhinAnim(delta_second);
	}

	if (invincible_time > 0.0f)
	{
		invincible_time -= delta_second;
		collision.object_type = eObjectType::eNone;
		collision.hit_object_type.clear();
	}
	else
	{
		// 自分のオブジェクトタイプ
		collision.object_type = eObjectType::ePlayer;
		// 当たる相手のオブジェクトタイプ
		collision.hit_object_type = { eObjectType::eEnemy, eObjectType::eExp, eObjectType::ePowerUp };
	}

	// 親クラスの更新処理を呼び出す
	__super::Update(delta_second);
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void Player::Draw(const Vector2D& screen_offset) const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, brend);

	if (is_alive)
	{
		if (is_shot_anim == true)
		{
			float position = location.x - 3.0f;

			if (stop == false)
			{
				if (powerd <= 1)
				{
					DrawRotaGraph(position, location.y - 20.0f, 1.0f, 0.0f, nozzle, TRUE);

				}
				else if (powerd == 2)
				{
					DrawRotaGraph(position + 10.0f, location.y - 0.0f, 1.5f, 0.0f, nozzle, TRUE);
					DrawRotaGraph(position - 10.0f, location.y - 0.0f, 1.5f, 0.0f, nozzle, TRUE);
				}
				else
				{
					DrawRotaGraph(position - 25.0f, location.y - 0.0f, 1.0f, 0.0f, nozzle, TRUE);
					DrawRotaGraph(position, location.y - 20.0f, 1.0f, 0.0f, nozzle, TRUE);
					DrawRotaGraph(position + 25.0f, location.y - 0.0f, 1.0f, 0.0f, nozzle, TRUE);
				}
			}
		}

		DrawRotaGraph(location.x - 10.0f, location.y + 70.0f, 1.0f, 0.0f, jet, TRUE);
		DrawRotaGraph(location.x + 10.0f, location.y + 70.0f, 1.0f, 0.0f, jet, TRUE);

		switch (anim_state)
		{
		case PlayerAnimState::Neutral:
			DrawRotaGraph(location.x, location.y, 1.0f, 0.0f, image, TRUE);
			break;
		case PlayerAnimState::TiltLeft:
			DrawRotaGraph(location.x, location.y, 1.0f, 0.0f, player_image_left[anim_index], TRUE);
			break;
		case PlayerAnimState::TiltRight:
			DrawRotaGraph(location.x, location.y, 1.0f, 0.0f, player_image_right[anim_index], TRUE);
			break;
		}

		if (is_shield == true)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
			DrawRotaGraph(location.x, location.y, 0.35f, 0.0f, shield, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		}

		// ライフ表示などはそのまま
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	#if _DEBUG
		if (is_attack_type)
			DrawString(location.x - 30.0f, location.y, "Attack Type", GetColor(255, 255, 255), TRUE);
		else
			DrawString(location.x - 35.0f, location.y, "Defence Type", GetColor(255, 255, 255), TRUE);
	#endif
	}
}

// 終了時処理
void Player::Finalize()
{
}

/// <summary>
/// ヒット時処理
/// </summary>
/// <param name="hit_object">当たった相手</param>
void Player::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::eEnemy ||
		hit_object->GetCollision().object_type == eObjectType::eEnemyShot ||
		hit_object->GetCollision().object_type == eObjectType::eEnemyBeam ||
		hit_object->GetCollision().object_type == eObjectType::eBoss)
	{
		if (on_hit == false)
		{
			if (is_shield == false)
			{
				life--;
				is_damage = true;
				Singleton<ShakeManager>::GetInstance()->StartShake(3.0f, 30, 20);
			}
			else
			{
				is_shield_damage = true;
				is_shield = false;
				shield_anim_on = false;
				shield_count = 0;
				shield_time = 0.0f;
			}
			on_hit = true;
		}
	}
	if (hit_object->GetCollision().object_type == eObjectType::ePowerUp)
	{
		if (powerd_on == false)
		{
			powerd++;
			powerd_on = true;
			powerd_time = 0.0f;
		}
	}
	if (hit_object->GetCollision().object_type == eObjectType::eShield)
	{
		is_shield = true;
	}
	if (hit_object->GetCollision().object_type == eObjectType::eMaxCharge)
	{
		charge = charge_max;
		charge_ready = true;
	}
}

/// <summary>
/// 移動処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
void Player::Movement(float delta_second)
{
	InputManager* input = Singleton<InputManager>::GetInstance();
	Vector2D stick_value = input->GetLeftStick();

	const float accel = 1000.0f;
	const float friction = 800.0f;
	const float dead_zone = 0.1f;
	const float max_speed = PLAYER_SPEED_MAX;

	// 入力ベクトル（初期化）
	Vector2D input_dir = { 0.0f, 0.0f };

	// スティック入力が有効ならそれを優先
	if (std::abs(stick_value.x) > dead_zone || std::abs(stick_value.y) > dead_zone) {
		input_dir.x = stick_value.x;
		input_dir.y = -stick_value.y;
	}
	else {
		if (input->GetKey(KEY_INPUT_D) || input->GetButton(XINPUT_BUTTON_DPAD_RIGHT)) input_dir.x += 1.0f;
		if (input->GetKey(KEY_INPUT_A) || input->GetButton(XINPUT_BUTTON_DPAD_LEFT))  input_dir.x -= 1.0f;
		if (input->GetKey(KEY_INPUT_S) || input->GetButton(XINPUT_BUTTON_DPAD_DOWN))  input_dir.y += 1.0f;
		if (input->GetKey(KEY_INPUT_W) || input->GetButton(XINPUT_BUTTON_DPAD_UP))    input_dir.y -= 1.0f;
	}

	// 入力方向があれば正規化
	if (input_dir.Length() > 1.0f) input_dir.Normalize();

	if (force_neutral_anim)
	{
		anim_state = PlayerAnimState::Neutral;
	}
	else
	{
		if (input_dir.x > 0.1f) {
			anim_state = PlayerAnimState::TiltRight;
		}
		else if (input_dir.x < -0.1f) {
			anim_state = PlayerAnimState::TiltLeft;
		}
		else {
			anim_state = PlayerAnimState::Neutral;
		}
	}

	// 仮の次の位置を計算
	Vector2D next_location = location + (input_dir * max_speed * delta_second);

	const float limit_x = (D_WIN_MAX_X / 2);
	const float half_width = 350.0f;

	// X軸の制限処理
	if (next_location.x < limit_x - half_width + 10) {
		input_dir.x = 0.0f;
		location.x = limit_x - half_width + 10;
	}
	else if (next_location.x > limit_x + half_width - 10) {
		input_dir.x = 0.0f;
		location.x = limit_x + half_width - 10;
	}

	// Y軸の制限処理
	if (next_location.y < 10) {
		input_dir.y = 0.0f;
		location.y = 10;
	}
	else if (next_location.y > D_WIN_MAX_Y - 10) {
		input_dir.y = 0.0f;
		location.y = D_WIN_MAX_Y - 10;
	}

	// 最終速度更新
	velocity = input_dir * max_speed;

	// 位置更新
	location += velocity * delta_second;
}

// 弾を打つ
void Player::Shot(float delta_second)
{
	// オブジェクト管理クラスのインスタンを取得
	InputManager* input = Singleton<InputManager>::GetInstance();
	SEManager* am = Singleton<SEManager>::GetInstance();

	// 打つまでの時間を計測
	shot_timer += delta_second;

	// プレイヤータイプの変更
	if (input->GetKeyDown(KEY_INPUT_LSHIFT) || 
		input->GetButtonDown(XINPUT_BUTTON_LEFT_SHOULDER) ||
		input->GetButtonDown(XINPUT_BUTTON_RIGHT_SHOULDER))
	{
		if (is_attack_type)
		{
			is_attack_type = false;
			image = defence;
			player_image_left = defence_player_image_left;
			player_image_right = defence_player_image_right;

			player_jet = defence_player_jet;

			nozzle_type = defence_nozzles;

			shot_interval = 0.15f;

			EffectManager* em =  Singleton<EffectManager>::GetInstance();
			effe_id = em->PlayerAnimation(EffectName::eAttackType, Vector2D(location.x, location.y + 10.0f), 0.04f, false);
			em->SetScale(effe_id, 0.35f);
		}
		else
		{
			is_attack_type = true;
			image = attack;
			player_image_left = attack_player_image_left;
			player_image_right = attack_player_image_right;

			player_jet = attack_player_jet;

			nozzle_type = attack_nozzles;

			shot_interval = 0.07f;

			EffectManager* em = Singleton<EffectManager>::GetInstance();
			effe_id = em->PlayerAnimation(EffectName::eDefenceType, Vector2D(location.x, location.y + 10.0f), 0.04f, false);
			em->SetScale(effe_id, 0.35f);
		}
	}

	EffectManager* em = Singleton<EffectManager>::GetInstance();
	em->SetPosition(effe_id, Vector2D(location.x, location.y + 10.0f));

	// スペースを長押ししたら一定間隔で発射
	if (input->GetKey(KEY_INPUT_SPACE) ||
		input->GetButton(XINPUT_BUTTON_A))
	{
		// 何も打ってなかったら打てるようにする
		if (stop == false)
		{
			// 発射インターバルを超えたら発射
			if (shot_timer >= shot_interval)
			{
				is_shot_anim = true;
				is_shot = true;
				shot_timer = 0.0f;
				am->PlaySE(SE_NAME::Shot);
				am->ChangeSEVolume(SE_NAME::Shot, 60);
			}
			GenarateBullet();
		}
	}
	else
	{
		is_shot_anim = false;
		// 押してない間は、即撃てるようにする
		shot_timer = shot_interval;
	}

#if _DEBUG
	// Bを押したらビーム発射
	if ((input->GetKeyDown(KEY_INPUT_B) || input->GetButtonDown(XINPUT_BUTTON_B)))  // チャージ完了時のみ
	{
		if (stop == false)
		{
			beam_on = true;
			stop = true;
			beam_timer = 0.0f;
			invincible_time = 5.0f;
			UseSpecial();  // ゲージ消費
			GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
			PlayerBeam* beam = gm->CreateObject<PlayerBeam>(Vector2D(location.x, (location.y - D_OBJECT_SIZE) - 848));
			beam->SetPlayer(this);
			SEManager::GetInstance()->PlaySE(SE_NAME::PlayerBeam);
			Singleton<ShakeManager>::GetInstance()->StartShake(5.0, 8, 8);
		}
	}
#else
	// Bを押したらビーム発射
	if ((input->GetKeyDown(KEY_INPUT_B) || input->GetButtonDown(XINPUT_BUTTON_B)) &&
		CanUseSpecial())  // チャージ完了時のみ
	{
		if (stop == false)
		{
			beam_on = true;
			stop = true;
			beam_timer = 0.0f;
			invincible_time = 5.0f;
			UseSpecial();  // ゲージ消費
			GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
			PlayerBeam* beam = gm->CreateObject<PlayerBeam>(Vector2D(location.x, (location.y - D_OBJECT_SIZE) - 848));
			beam->SetPlayer(this);
			SEManager::GetInstance()->PlaySE(SE_NAME::PlayerBeam);
		}
	}
#endif

	// ５秒経ったらビームの再起
	if (beam_timer >= 5.0f)
	{
		stop = false;
		beam_on = false;
	}
	else
	{
		beam_timer += delta_second;
	}
}

void Player::Damage(float delta_second)
{
	// ダメージを受けたときチカチカさせる
	if (is_damage == true)
	{
		// カウント加算
		damage_timer += delta_second;
		if (damage_timer >= 0.05f)
		{
			// 到達カウントを加算
			reach_count++;
			// 到達カウントが偶数であれば透明に
			if (reach_count % 2 == 0)
			{
				brend = 0;
			}
			// 奇数であれば不透明に
			else
			{
				brend = 255;
			}
			//カウントリセット
			damage_timer = 0;
		}
		// チカチカが終わったら全てリセット
		if (reach_count >= 30)
		{
			is_damage = false;
			on_hit = false;
			brend = 255;
			damage_timer = 0;
			reach_count = 0;
		}
	}

	if (is_shield_damage)
	{
		shield_damage_timer += delta_second;
		if (shield_damage_timer >= 1.0f)
		{
			is_shield_damage = true;
			on_hit = false;
			shield_damage_timer = 0.0f;
		}
	}
}

void Player::BuhinAnim(float delta_second)
{
	// アニメーション更新処理
	anim_timer += delta_second;

	if (anim_timer >= anim_interval)
	{
		anim_timer = 0.0f;
		anim_index++;
		if (anim_index >= 2) anim_index = 1;
	}

	std::vector<int> animation_num = { 3, 4, 5, 4 };
	//フレームレートで時間を計測
	animation_time += delta_second;
	//8秒経ったら画像を切り替える
	if (animation_time >= 0.02f)
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
		jet = player_jet[animation_num[animation_count]];
	}

	std::vector<int> nozzle_num = nozzle_type;
	//フレームレートで時間を計測
	nozzle_time += delta_second;
	//8秒経ったら画像を切り替える
	if (nozzle_time >= 0.0125f)
	{
		//計測時間の初期化
		nozzle_time = 0.0f;
		//時間経過カウントの増加
		nozzle_count++;
		//カウントがアニメーション画像の要素数以上になったら
		if (nozzle_count >= nozzle_num.size())
		{
			//カウントの初期化
			nozzle_count = 0;
		}
		// アニメーションが順番に代入される
		nozzle = nozzles[nozzle_num[nozzle_count]];
	}

	if (is_shield == true)
	{
		std::vector<int> shield_num;

		if (shield_anim_on == false)
		{
			shield_num = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

			// フレームレートで時間を計測
			shield_time += delta_second;

			// 一定時間経ったら画像を切り替える
			if (shield_time >= 0.05f)
			{
				// 計測時間の初期化
				shield_time = 0.0f;

				// 時間経過カウントの増加
				shield_count++;

				// カウントがアニメーション画像の要素数以上になったら
				if (shield_count >= shield_num.size())
				{
					// 次の段階へ移行
					shield_count = 0;  // ← -1にして次フレームで0になるよう調整
					shield_anim_on = true;
				}
				else
				{
					// アニメーションが順番に代入される
					shield = shields2[shield_num[shield_count]];
				}
			}
		}
		else
		{
			shield_secand = true;
			shield_num = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

			// フレームレートで時間を計測
			shield_time += delta_second;

			// 一定時間経ったら画像を切り替える
			if (shield_time >= 0.0125f)
			{
				// 計測時間の初期化
				shield_time = 0.0f;

				shield_count++;

				if (shield_count >= shield_num.size())
				{
					shield_count = 0;
				}
				else
				{
					// アニメーションが順番に代入される
					shield = shields[shield_num[shield_count]];
				}
			}
		}
	}
	else
	{
		shield = shields2[0];
	}
}

void Player::GenarateBullet()
{
	// プレイヤーが弾を打つ準備ができていたら弾を生成        
	if (is_shot == true)
	{
		is_shot = false;
		GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

		if (is_attack_type == true)
		{
			// 上方向に生成
			if (powerd <= 1)
			{
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
			}
			else if (powerd == 2)
			{
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 30, location.y));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 30, location.y));
			}
			else
			{
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 50, location.y + D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 30, location.y));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x - 30, location.y));
				objm->CreateObject<PlayerAttackBullet>(Vector2D(location.x + 50, location.y + D_OBJECT_SIZE));
			}
		}
		else
		{
			// 上方向に生成
			if (powerd <= 1)
			{
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
			}
			else if (powerd == 2)
			{
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 30, location.y));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 30, location.y));
			}
			else
			{
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 50, location.y + D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 30, location.y));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 10, location.y - D_OBJECT_SIZE));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x - 30, location.y));
				objm->CreateObject<PlayerDefenceBullet>(Vector2D(location.x + 50, location.y + D_OBJECT_SIZE));
			}

		}
	}
}

bool Player::GetIsAlive() const
{
	return is_alive;
}

bool Player::GetBeamOn() const
{
	return beam_on;
}

bool Player::GetShotFlip() const
{
	return shot_flip;
}

void Player::SetBeamOn()
{
	beam_on = false;
}

void Player::AddCharge(float value)
{
	// ビーム中は加算しない
	if (beam_on) return;

	charge += value;
	if (charge >= charge_max)
	{
		charge = charge_max;
		charge_ready = true;
	}
}

bool Player::CanUseSpecial() const
{
	return charge_ready;
}

void Player::UseSpecial()
{
	charge = 0.0f;
	charge_ready = false;
}

float Player::GetChargeRate() const
{
	// 発動中は常にMAX表示
	if (beam_on) return 1.0f;
	return charge / charge_max;
}

int Player::GetPowerd() const
{
	return powerd;
}

bool Player::GetShieldOn() const
{
	return is_shield;
}
int Player::GetLife() const
{
	return life;
}
void Player::SetShotStop(bool stoping)
{
	shot_stop = stoping;
}
bool Player::GetGameOver() const
{
	return game_over_player;
}
void Player::ForceNeutralAnim(bool enable)
{
	force_neutral_anim = enable;
}
