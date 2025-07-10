#pragma once
#include "../GameObjectBase.h"
class EnemyBullet2 :
	public GameObjectBase
{
private:
	int se = NULL;
	float move_time = 0.0f;

private:
	Vector2D suck_center;

public:
	bool is_wave_reflected = false; // 波の向きが反転しているか

private:
	enum class ShotState
	{
		SpreadOut,
		PullIn,
		Explode
	};
	ShotState shot_state = ShotState::SpreadOut;

private:
	Vector2D base_velocity;     // 前進方向の速度
	float wave_amplitude;       // 波の振れ幅
	float wave_frequency;       // 波の周波数

public:
	EnemyBullet2();
	~EnemyBullet2();

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

public:
	void SetSuckCenter(const Vector2D& center);
	void SetWaveParameters(float amplitude, float frequency);
	void SetWaveReflected(bool reflected); // 設定関数
};

