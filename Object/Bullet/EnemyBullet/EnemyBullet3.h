#pragma once
#include "EnemyBulletBase.h"
class EnemyBullet3 :
    public EnemyBulletBase
{
private:
	int attack_pattrn = 0;
	int input_count = 0;
	float move_time = 0.0f;

	std::vector<int> bullet;
	std::vector<int> bullet_1;
	std::vector<int> bullet_2;
	std::vector<int> bullet_3;

	std::vector<std::vector<int>> bullets;

public:
	EnemyBullet3();
	~EnemyBullet3();

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

	void SetAttackPattrn(int pattrn);
};

