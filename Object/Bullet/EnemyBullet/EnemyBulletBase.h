#pragma once
#include "../../GameObjectBase.h"
class EnemyBulletBase :
    public GameObjectBase
{
protected:
	class Player* player;

public:
	EnemyBulletBase();
	~EnemyBulletBase();

public:
	// 初期化処理
	virtual void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	/// <returns></returns>
	virtual void Update(float delata_second) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="delta_second"></param>
	/// <returns></returns>
	virtual void Draw(const Vector2D& screen_offset)const override;

	// 終了時処理
	virtual void Finalize() override;

public:
	/// <summary>
	/// ヒットしたときの処理
	/// </summary>
	/// <param name="hit_object">プレイヤーだとしたら、ここは敵とかブロックと当たったことになる</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;

	void SetPlayer(Player* p);

protected:
	void DropItems();  // ← 追加

};

