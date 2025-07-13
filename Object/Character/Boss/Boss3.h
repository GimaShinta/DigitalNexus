#pragma once
#include "../Enemy/EnemyBase.h"
//#include "../../Shot/EnemyShot/EnemyShot3.h"
#include "../../Beam/EnemyBeam.h"

const int MAX_ATTACK_PATTRN = 8;

class Boss3 : public EnemyBase
{
private:
	int se_beam;

	int attack_pattrn = 0;
	int prev_attack_pattrn = -1; // 前フレームのパターン記録用

	std::vector<int> attack_pattrn_num;
	int attack_count = 0;

	bool generate = false;
	bool generate2 = false;
	bool is_alive = true;
	bool is_collision;

	float move_time = 0.0f; // ∞字用の時間変数
	Vector2D base_position; // 軌道の中心位置
	Vector2D generate_base_position; // 軌道の中心位置

	bool swinging_right = true;      // 現在右に向かってるかどうか
	float swing_timer = 0.0f;        // 静止中のタイマー
	bool is_pausing = false;         // 静止中かどうか

	float image_size = 1.0f;

	int Boss3_image[8] = { 0 };
	std::vector<int> Boss3_anim;
	float anim_speed = 0.1f;

	// 部品の位置（左右それぞれ2個ずつ）
	Vector2D part_positions[6]; // 0:左奥, 1:左手前, 2:右手前, 3:右奥
	float angle = 0.0f;

	// クラス Boss3 に以下の変数を追加

	bool has_initialized_swing_x = false;
	float swing_center_x = 0.0f;
	int swing_direction = 1;           // 1 = 右, -1 = 左
	float swing_wait_timer = 0.0f;     // 停止中のタイマー

	class EnemyBullet3* e_shot4;
	class EnemyBullet4* e_shot5;

	std::vector<int> Boss3_jet;
	int jet;

	float jet_timer = 0.0f;
	float jet_count = 0;


	float hpbar_fade_timer = 0.0f;  // フェード用タイマー
	bool show_hpbar = false;        // HPバーを表示するか

	float damage_timer = 0.0f;

	bool is_weakness = false;

	bool is_crashing = false; // 墜落中かどうか
	float crash_timer = 0.0f; // 墜落開始からの時間

	int explosion_index = 0;
	float explosion_timer = 0.0f;
	const float explosion_interval = 0.2f; // 爆発の間隔（秒）
	const int max_explosions = 23;          // 爆発の最大数
	bool explosions_started = false;       // 爆発処理を開始したかどうか
	float fall_speed_y = 0.0f; // 落下速度（初期は0）
	Vector2D crash_velocity = Vector2D(100.0f, -200.0f); // 初速度（右へ + 上に少し浮いてから落下）
	float gravity = 200.0f; // 重力加速度（Y方向）

	struct Ripple {
		Vector2D pos;
		float timer;
		bool active;
	};

	Ripple ripples[5]; // 最大5つ

	struct RotatingShotData
	{
		EnemyBullet3* shot;
		float angle_deg;
		Vector2D target_offset; // 中心からのオフセット（目標位置）
		float arrival_timer;
		enum class State { MoveToCircle, Rotate } state;
		Vector2D fixed_offset; // 固定された中心位置

	};

	float ripple_spawn_timer = 0.0f;
	int ripple_spawn_count = 0;
	bool ripple_start = false; // 波紋出現を開始するフラグ

	int second_attack = 0;
	int se[3];
	std::map<int, std::pair<Vector2D, Vector2D>> ripple_positions;
	bool is_drive = false;
	bool is_shot = false;
	float shot_timer = 0.0f;
	float delta = 0.0f;
	float beam_damage_timer = 0.0f;
	bool on_hit = false;

public:
	Boss3();
	~Boss3();

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

	void OnHitCollision(GameObjectBase* hit_object) override;

protected:
	/// <summary>
	/// 移動処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	void Movement(float delta_second) override;

	void Shot(float delta_second);

private:
	void DrawBoss3(const Vector2D position) const;

public:
	int GetAttackPattrn() const;
	int GetIsAlive() const;

	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	float Max(float a, float b)
	{
		return (a > b) ? a : b;
	}
	float Min(float a, float b)
	{
		return (a < b) ? a : b;
	}

	//template <typename T>
	//T Lerp(T a, T b, float t)
	//{
	//	return a + (b - a) * t;
	//}
	int ripple_attack_pattrn = 0;
	bool GetGenerate() const;
	bool GetIsCrashing() const;
	bool IsBoss() const override { return true; }
	//template <typename T>
	//T Clamp(T value, T min, T max) {
	//	return (value < min) ? min : (value > max) ? max : value;
	//}

	class Player* player = nullptr;

public:
	void SetPlayer(Player* p) { player = p; }

private:
	void Attack(float delta_second);

	void Pattrn4(int bullet_num, float speed, float spiral_interval, float spiral_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn4_2(int bullet_num, float speed, float spiral_interval, float spiral_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn5(float spiral_interval, float spiral_duration_limit, float spiral_speed, const Vector2D& generate_location, float delta_second);
	void Pattrn5_2(float spiral_interval, float spiral_duration_limit, float spiral_speed, const Vector2D& generate_location, float delta_second);
	void Pattrn6(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn6_2(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn7(int bullet_num, float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second, bool image_change);
	void Pattrn7_2(float fan_angle_range, float bullet_speed, float fan_interval, float fan_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn8(float wave_interval, float wave_duration_limit, const Vector2D& generate_location, float delta_second);
	void Pattrn9(int shot_count, float radius, float angular_speed, float bullet_speed, const Vector2D& generate_location, float delta_second);
	void Pattrn9_2(int shot_count, float radius, float angular_speed, float bullet_speed, const Vector2D& generate_location, float delta_second);
	void Pattrn10(int shot_count, float radius, float angular_speed, float center_speed, float duration_limit, const Vector2D& center_location, float delta_second);
	void Pattrn10_2(int shot_count, float radius, float angular_speed, float center_speed, float duration_limit, const Vector2D& center_location, float delta_second);
	void Pattrn11(float offsets_x);
	void Pattrn12();
	void Pattrn13(float delta_second);
};

