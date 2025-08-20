#pragma once
#include "EnemyBulletBase.h"
class EnemyBullet5 : public EnemyBulletBase
{
private:
	Vector2D base_velocity = { 0.0f, 0.0f };  // 進行方向（通常は真下など）
	float amplitude = 0.0f;                 // 波の振幅（大きさ）
	float frequency = 1.0f;                 // 周波数（波のスピード）
	bool reverse = false;                   // 波の向きを左右反転するか
	float timer = 0.0f;                     // 経過時間（sin波に使う）

public:
	EnemyBullet5();
	~EnemyBullet5();

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
	void SetWaveParameters(const Vector2D& base_velocity, float amplitude, float frequency, bool reverse);
};

