#pragma once
#include "../GameObjectBase.h"
#include"../../Object/Character/Player/Player.h"

class ItemBase : public GameObjectBase
{
protected:
	Player* player;	//共有されたプレイヤー情報

	float attract_range = 250.0f;   //追従をONにする距離
	bool is_attracting = false;    //当たっているか

	float speed = 1.0f;           // 初期スピード
	float max_speed = 200.0f;       // 最大スピード
	float acceleration = 100.0f;    // 加速度（1秒あたりの増加量）

	std::vector<int> images;

	bool is_collected = false;

public:
	// 初期化処理
	virtual void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	/// <returns></returns>
	virtual void Update(float delata_second);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="delta_second"></param>
	/// <returns></returns>
	virtual void Draw(const Vector2D& screen_offset)const;

	// 終了時処理
	virtual void Finalize();

public:
	/// <summary>
	/// ヒットしたときの処理
	/// </summary>
	/// <param name="hit_object">プレイヤーだとしたら、ここは敵とかブロックと当たったことになる</param>
	virtual void OnHitCollision(GameObjectBase* hit_object);

	/// <summary>
	/// プレイヤー情報を共有
	/// </summary>
	/// <param name="p">プレイヤー情報</param>
	void SetPlayer(Player* p);
};

