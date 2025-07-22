#pragma once

#include "../Utility/Vector2D.h"
#include "../Utility/BoxCollision.h"
#include"../Utility/ResourceManager.h"
#include "../Utility/ProjectConfig.h"
#include "../Utility/EffectManager.h"
#include "../Utility/SEManager.h"
#include "../Utility/ShakeManager.h"
#include "DxLib.h"

#define D_OBJECT_SIZE	(24.0f)			// オブジェクトサイズ（共通）

// オブジェク基底クラス
class GameObjectBase
{
protected:
	class GameObjectManager* owner_scene;   //
	Vector2D location;                      // オブジェクトのウィンドウの位置座標
	Vector2D old_location;                      // オブジェクトのウィンドウの位置座標
	BoxCollision collision;                 // 当たり判定情報の保持
	unsigned char z_layer;          //

public:
	Vector2D box_size;              // 当たり判定サイズ
	Vector2D velocity;              // 速度


protected:
	int image;                      // 画像用
	std::vector<int> images;
	int animation_count;            // 一定時間の到達数のカウント
	float animation_time;           // アニメーションの間隔
	int score;

public:
	bool is_mobility;               // 動くかどうか（trueなら動く、falseなら止まる）
	bool is_destroy;                // 死んでいるかどうか

public:
	GameObjectBase();
	virtual ~GameObjectBase();

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
	/// アニメーション制御
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	/// <param name="animation_image">アニメーション総画像</param>
	/// <param name="animation_num">アニメーション順序</param>
	virtual void AnimationControl(float delta_second, std::vector<int>& animation_image, std::vector<int> animation_num, float chenge_time);
	/// <summary>
	/// 画像の設定
	/// </summary>
	/// <param name="animation_imag">描画したい単体画像</param>
	virtual void AnimationControl(int animation_image);

public:
	void SetOwnerScene(class GameObjectManager* scene); // 
	void SetLocation(const Vector2D& location);         // 位置座標を設定する（主にオブジェクトの生成位置を決める）
	Vector2D& GetLocation();                            // 現在の位置座標を取得する

	Vector2D& GetVelocity();	//速度取得
	void SetVelocity(const Vector2D& velo);//速度設定
	void SetScroll(float scroll);	// スクロール量の設定
	void SetDestroy();	// スクロール量の設定

	virtual bool IsBoss() const { return false; };


	Vector2D& GetBoxSize();                             // オブジェクトの大きさを取得する
	const BoxCollision& GetCollision()const;            // オブジェクトがもつ当たり判定情報（左上と右下の点とか）を取得する
	const unsigned char GetZLayer()const;
	const bool& GetMobility()const;                     // 
	const bool& Getblokking()const;                     // 
	// clearしているか
	int GetScore();
};

