#pragma once
#include "../CharacterBase.h"
#include <algorithm>

const int BULLET_MAX = 100; // 自機が発射する弾の最大数
const int PLAYER_SHIELD_MAX = 8; // 自機のシールドの最大値
const int WEAPON_LV_MAX = 10; // 武器レベルの最大値
const int PLAYER_SPEED_MAX = 450; // 自機の速さの最大値
const float SHOT_INTERVAL = 0.07f; // 0.25秒ごとに撃つ
const float BEAM_TIME = 5.0f; // 0.25秒ごとに撃つ
const float beam_duration = 5.0f; // 5秒間

enum
{
	LEVEL1 = 1,
	LEVEL2,
	LEVEL3
};

enum class PlayerType
{
	AlphaCode,
	OmegaCode
};

class Player : public CharacterBase
{
private:
	enum class PlayerAnimState
	{
		Neutral,   // 正面
		TiltLeft,  // 左傾き
		TiltRight  // 右傾き
	};


	PlayerType now_type = PlayerType::AlphaCode;

	PlayerAnimState anim_state = PlayerAnimState::Neutral;
	float anim_timer = 0.0f;
	int anim_index = 0;
	float anim_interval = 0.1f;

public:
	int life;
private:
	bool is_shot = false;
	float shot_timer = 0.0f;
	bool on_hit = false;
	bool beam_on = false;
	bool stop = false;
	bool is_damage = false;
	bool shot_flip = false;
	int reach_count = 0;
	int brend = 255;
	float beam_timer = 0.0f;
	float damage_timer = 0.0f;
	float shield_damage_timer = 0.0f;
	bool is_shield_damage = false;
	// 追加：シールドの切り替え直後1フレームスキップ用フラグ
	bool just_switched_to_second = false;
	int powerd = 2;
	bool powerd_on = false;
	float powerd_time = 0.0f;
	bool is_alive = true;
	bool is_shield = false;
	bool shield_secand = false;

	bool is_dead_animation_playing = false;
	float dead_animation_timer = 0.0f;    // 演出再生用タイマー
	const float dead_animation_duration = 1.5f; // 例：2秒間の死の演出

private:
	float charge = 0.0f;
	const float charge_max = 100.0f;
	bool charge_ready = false;

	float nozzle_time = 0.0f;
	int nozzle_count = 0;

	bool is_shot_anim = false;
	int se_shot = NULL;

	float shield_time = 0.0f;
	int shield_count = 0;

	bool shield_anim_on = false;

	bool shot_stop = false;

	// 画像系
	std::vector<int> player_image_right;
	std::vector<int> player_image_left;
	std::vector<int> attack_player_image_right;
	std::vector<int> attack_player_image_left;
	std::vector<int> defence_player_image_right;
	std::vector<int> defence_player_image_left;
	std::vector<int> attack_player_jet;
	std::vector<int> defence_player_jet;
	std::vector<int> player_jet;
	std::vector<int> attack_nozzles;
	std::vector<int> defence_nozzles;
	std::vector<int> nozzles;
	std::vector<int> nozzle_type;
	std::vector<int> shields;
	std::vector<int> shields2;
	int nozzle = NULL;
	int jet = NULL;
	int shield = NULL;
	int attack = NULL;
	int defence = NULL;

	float last_explosion_time = -999.0f; // 最後に爆発を再生した時間
	float invincible_time = 0.0f;

	bool game_over_player = false;

	float shot_interval = 0.0f;

	int effe_id = 0;

	class PlayerDefenceBullet* defence_bullet = nullptr;

	bool can_change_type = true;

public:
	Player();
	~Player();

public:
	// 初期化処理
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delata_second">１フレーム当たりの時間</param>
	void Update(float delta_second) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="screen_offset"></param>
	void Draw(const Vector2D& screen_offset)const override;

	// 終了時処理
	void Finalize() override;

public:
	/// <summary>
	/// ヒット時処理
	/// </summary>
	/// <param name="hit_object">当たった相手</param>
	void OnHitCollision(GameObjectBase* hit_object) override;

private:
	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	void Movement(float delta_second) override;

	void Shot(float delta_second);

	void Damage(float delta_second);

	void BuhinAnim(float delta_second);

	void GenarateBullet();

public:
	bool GetIsAlive() const;
	bool GetBeamOn() const;
	bool GetShotFlip() const;
	void SetBeamOn();

public:
	void AddCharge(float value);
	bool CanUseSpecial() const;
	void UseSpecial();
	float GetChargeRate() const;
	int GetPowerd() const;
	bool GetShieldOn() const;
	int GetLife() const;
	void SetShotStop(bool stoping);
	bool GetGameOver() const;
	PlayerType GetNowType() const;

public:
	void SetLocation(const Vector2D& pos) { location = pos; }
	void SetMobility(bool enable) { is_mobility = enable; }

	void ForceNeutralAnim(bool enable); // 強制ニュートラル用

	void SetCanChangeType(bool enable);

private:
	bool force_neutral_anim = false;


	float Min(float a, float b)
	{
		return (a < b) ? a : b;
	}

};

