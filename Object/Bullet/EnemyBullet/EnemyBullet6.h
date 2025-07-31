#pragma once
#include "../../GameObjectBase.h"
class EnemyBullet6 :
    public GameObjectBase
{
private:
	enum Phase { MoveIn, Stop, Homing, Straight };
	Phase phase = MoveIn;

	float timer = 0.0f;
	float stop_duration = 1.0f;
	float homing_speed = 200.0f;
	float initial_speed = 400.0f;
	Vector2D last_velocity;
	Vector2D homing_velocity;

	float total_move_duration = 0.8f; // 前進にかかる時間（減速時間）
	float total_homing_duration = 0.6f; // 追尾にかける加速時間
	float target_stop_y = 0.0f;
	float homing_end_distance = 200.0f; 
	class Player* player;
	Vector2D homing_direction;
	Vector2D target_stop_position;

public:
	EnemyBullet6();
	~EnemyBullet6();

public:
	// 初期化処理
	virtual void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delata_second">１フレーム当たりの時間</param>
	virtual void Update(float delta_second) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

	// 終了時処理
	virtual void Finalize() override;

	/// <summary>
	/// ヒットしたときの処理
	/// </summary>
	/// <param name="hit_object">プレイヤーだとしたら、ここは敵とかブロックと当たったことになる</param>
	void OnHitCollision(GameObjectBase* hit_object) override;

	void SetPlayer(class Player* p);

	// 線形補間
	float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}
	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}


	void SetPhaseParams(float init_speed, float stop_dur, float homing_spd, float stop_x_random_offset, float stop_y_random_offset);
};

