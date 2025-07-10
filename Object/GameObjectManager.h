#pragma once

#include "../Singleton.h"
#include "GameObjectBase.h"

#include <vector>
#include <string>

// オブジェクトの生成と破棄を管理するクラス
class GameObjectManager : public Singleton<GameObjectManager>
{
private:
	std::vector<GameObjectBase*> create_object;     // 生成したいオブジェクトが入る配列（生成配列）
	std::vector<GameObjectBase*> destroy_object;	// 破棄したいオブジェクトが入る配列（削除配列）
	std::vector<GameObjectBase*> game_object;	    // インゲームに存在するオブジェクトが入る配列（インゲーム配列）

	Vector2D screen_offset;// スクリーンオフセットはウィンドウの0,0位置からどれだけ離れているのかを判定するのに使える

public:
	GameObjectManager();
	virtual ~GameObjectManager();

public:
	// インスタンス（オブジェクトへの参照）の削除
	static void DeleteInstance();
	const std::vector<GameObjectBase*>& GetGameObjects() const
	{
		return game_object;
	}

public:
	// 初期化処理
	virtual void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	/// <returns></returns>
	virtual void Update(const float& delta_second);

	// 描画処理
	virtual void Draw() const;

	// 終了時処理
	virtual void Finalize();

	// 生成できるオブジェクトがあるかをチェック
	virtual void CheckCreateObject();

	// 削除できるオブジェクトがあるかをチェック
	virtual void CheckDesroyObject();

	//virtual std::vector<GameObjectBase*>& CreateGameObject();

	/// <summary>
	/// オブジェクトの生成
	/// </summary>
	/// <typeparam name="OBJECT">GameObjectBaseを継承したクラス</typeparam>
	/// <param name="generate_location">生成する位置座標</param>
	/// <returns>生成したオブジェクトのインスタンス（オブジェクトへの参照）</returns>
	template <class OBJECT>
	OBJECT* CreateObject(const Vector2D& generate_location)
	{
		// 生成するゲームオブジェクトクラスを動的確保
		OBJECT* new_instance = new OBJECT();
		// GameObjectBaseを継承しているか確認
		GameObjectBase* new_object = dynamic_cast<GameObjectBase*>(new_instance);
		// エラーチェック
		if (new_object == nullptr)
		{
			// 動的確保したメモリの開放
			delete new_instance;
			// 例外テキストを通知
			throw std::string("ゲームオブジェクトが生成できませんでした");
			return nullptr;
		}
		// シーン情報の設定
		new_object->SetOwnerScene(this);
		// 生成されたオブジェクトの初期化処理をする
		new_object->Initialize();
		// 生成されたオブジェクトの位置情報を設定
		new_object->SetLocation(generate_location);
		// 生成配列の末尾に格納する
		create_object.push_back(new_object);
		// 生成したオブジェクトのポインタを返す
		return new_instance;
	}

	/// <summary>
	/// オブジェクトを削除する
	/// </summary>
	/// <param name="target">削除したいオブジェクト</param>
	void DestroyGameObject(GameObjectBase* target);

public:
	/// <summary>
	/// 当たり判定のチェック
	/// </summary>
	/// <param name="target">プレイヤー</param>
	/// <param name="partner">敵とかブロックとか</param>
	virtual void CheckCollision(GameObjectBase* target, GameObjectBase* partner);

	// 
	const Vector2D GetScreenOffset() const;
	void DrawBoss() const;
	void DrawWithoutBoss() const;

private:
	// インゲームに存在する全てのオブジェクトを削除
	void DestoryAllObject();

};
