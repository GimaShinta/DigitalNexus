#pragma once
#include "../GameObjectBase.h"
class EnemyBullet1 :
    public GameObjectBase
{
private:
	int se = NULL;
	float move_time = 0.0f;

public:
	EnemyBullet1();
	~EnemyBullet1();

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

};

