#pragma once
#include "../CharacterBase.h"
#include <algorithm>

const int BULLET_MAX = 100;			// 自機が発射する弾の最大数
const int PLAYER_SHIELD_MAX = 8;	// 自機のシールドの最大値
const int WEAPON_LV_MAX = 10;		// 武器レベルの最大値
const int PLAYER_SPEED_MAX = 450;	// 自機の速さの最大値
const float SHOT_INTERVAL = 0.07f;  // 0.07秒ごとに撃つ
const float BEAM_TIME = 5.0f;		// 5.0秒間撃つ
const float beam_duration = 5.0f;	// 5秒間

//プレイヤーの攻撃形態
enum class PlayerType
{
	AlphaCode,	//攻撃
	OmegaCode	//相殺
};

class Player : public CharacterBase
{

private:

	//傾きアニメーション
	enum class PlayerAnimState
	{
		Neutral,   // 正面
		TiltLeft,  // 左傾き
		TiltRight  // 右傾き
	};

	//プレーヤーの初期形態
	PlayerType now_type = PlayerType::AlphaCode;

	//アニメーション関連
	PlayerAnimState anim_state = PlayerAnimState::Neutral;
	float anim_timer = 0.0f;
	int anim_index = 0;
	float anim_interval = 0.1f;

public:

	//ライフ：外部参照のためパブリック
	int life;

private:

	//射撃・状態
	bool is_shot = false;
	bool on_hit = false;
	bool beam_on = false;
	bool specialmove_use = false;
	bool recovery_on = false;
	bool stop = false;
	bool is_damage = false;
	bool shot_flip = false;
	bool is_shield_damage = false;

	float shot_timer = 0.0f;
	float beam_timer = 0.0f;
	float damage_timer = 0.0f;
	float shield_damage_timer = 0.0f;

	int reach_count = 0;
	int brend = 255;

	//シールドの切り替え直後1フレームスキップ用
	bool just_switched_to_second = false;

	//パワー調整用
	int powerd = 1;
	bool powerd_on = false;
	float powerd_time = 0.0f;

	//生存しているか・シールド
	bool is_alive = true;
	bool is_shield = false;
	bool shield_secand = false;

	//ゲームオーバー演出用変数
	bool is_dead_animation_playing = false;
	float dead_animation_timer = 0.0f;			// 演出再生用タイマー
	const float dead_animation_duration = 1.5f; // 〇〇秒間のゲームオーバーの演出

private:
	//チャージ(アルファ/オメガ)
	float charge = 0.0f;
	const float charge_max = 100.0f;
	bool charge_ready = false;

	float charge2 = 0.0f;
	const float charge_max2 = 100.0f;
	bool charge2_ready = false;

	//ノズル：シールド
	int nozzle_count = 0;
	int se_shot = NULL;
	int shield_count = 0;

	float nozzle_time = 0.0f;
	float shield_time = 0.0f;

	bool is_shot_anim = false;
	bool shield_anim_on = false;
	bool shot_stop = false;

	// 画像系:アセット
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

	class PlayerDefenceBullet* defence_bullet = nullptr;

	int nozzle = NULL;
	int jet = NULL;
	int shield = NULL;
	int attack = NULL;
	int defence = NULL;
	int bottan[2] = {0};

	int effe_id = 0;
	int on_count = 0;

	float last_explosion_time = -999.0f; // 最後に爆発を再生した時間
	float invincible_time = 0.0f;
	float shot_interval = 0.0f;
	float bottan_srid = 0.0f;

	bool can_change_type = true;
	bool can_change_type_now = true;
	bool game_over_player = false;
	bool bottan_ok = false;


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

	/// <summary>
	/// ショット
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	void Shot(float delta_second);

	/// <summary>
	/// ダメージ
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	void Damage(float delta_second);

	/// <summary>
	/// 部品アニメーション
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	void BuhinAnim(float delta_second);

	/// <summary>
	/// 弾生成
	/// </summary>
	void GenarateBullet();

	/// <summary>
	/// ショットSE設定
	/// </summary>
	void PlayShotSE(); 


public:
	//プレーヤー状態取得
	bool GetIsAlive() const;
	bool GetBeamOn() const;
	bool GetRecoveryOn() const;
	bool GetShotFlip() const;
	void SetBeamOn();

public:
	//チャージ・特殊系
	void AddCharge(float value);
	void AddCharge2(float value);
	bool CanUseSpecial() const;
	bool CanUseSpecial2() const;
	void UseSpecial();
	void UseSpecial2();
	float GetChargeRate() const;
	float GetCharge2Rate() const;

	//パワー・UI系
	int GetPowerd() const;
	bool GetShieldOn() const;
	int GetLife() const;
	void SetShotStop(bool stoping);
	bool GetGameOver() const;
	bool GetSpecialMove() const;

	//プレイヤーの攻撃タイプ
	PlayerType GetNowType() const;
	void SetNowType(PlayerType nt);
	void ChangeType(PlayerType new_type, bool play_effect);

public:
	//ユーティリティ：座標系
	void SetLocation(const Vector2D& pos) { location = pos; }
	void SetMobility(bool enable) { is_mobility = enable; }

	//強制ニュートラル
	void ForceNeutralAnim(bool enable); 

	//攻撃タイプ切替許可
	void SetCanChangeType(bool enable);

	//プレイヤー識別
	bool IsPlayer() const override { return true; }

private:
	bool force_neutral_anim = false;

	float Min(float a, float b)
	{
		return (a < b) ? a : b;
	}

};

