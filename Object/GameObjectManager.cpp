#include "GameObjectManager.h"
#include "../Application/Application.h"
#include "../Utility/BoxCollision.h"
#include "../Utility/ProjectConfig.h"

GameObjectManager::GameObjectManager()
{
}

GameObjectManager::~GameObjectManager()
{
	// 解放忘れ防止
	Finalize();
}

// インスタンス（オブジェクトへの参照）の削除
void GameObjectManager::DeleteInstance()
{
	// 自クラスのポインタ（実体をアドレスの先で保有）
	static GameObjectManager* instance = nullptr;

	// インスタンスが存在している場合、削除する
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

// 初期化処理
void GameObjectManager::Initialize()
{
	screen_offset.x = D_WIN_MAX_X / 2;
	screen_offset.y = D_WIN_MAX_Y / 2;
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
void GameObjectManager::Update(const float& delta_second)
{
	// 生成できるオブジェクトをゲーム配列に移動させる
	CheckCreateObject();

	// 削除フラグがオンだったら削除
	for (int i = 0; i < game_object.size(); i++)
	{
		if (game_object[i]->is_destroy == true)
		{
			DestroyGameObject(game_object[i]);
		}
	}

	// 削除されるオブジェクトを配列に入れる
	CheckDesroyObject();

	// インゲーム配列内のオブジェクトを更新する
	for (GameObjectBase* obj : game_object)
	{
		// ゲームオブジェクトの更新処理を開始する（動作が重くなるのを防ぐ）
		obj->Update(delta_second);
	}

	// 当たり判定確認処理
	for (int i = 0; i < game_object.size(); i++)
	{
		// 移動の許可
		if (game_object[i]->GetMobility() == false)
		{
			continue;
		}

		// 見ているオブジェクトが同じであれば当たり判定を無くす
		for (int j = 0; j < game_object.size(); j++)
		{
			if (i == j)
			{
				continue;
			}

			// プレイヤーとエネミーが当たっているかをチェック
			CheckCollision(game_object[i], game_object[j]);
		}
	}
}

// 描画処理
void GameObjectManager::Draw() const
{
	// ゲーム配列内のオブジェクトを描画する
	for (GameObjectBase* obj : game_object)
	{
		// ゲームオブジェクトの描画処理を開始する（動作が重くなるのを防ぐ）
		obj->Draw(screen_offset);
	}
}

// 終了時処理（主にオブジェクトリスト）
void GameObjectManager::Finalize()
{
	// オブジェクトリスト内のオブジェクトを破棄する
	DestoryAllObject();

	// 動的配列の解放
	if (!create_object.empty())
	{
		create_object.clear();
	}
	if (!destroy_object.empty())
	{
		destroy_object.clear();
	}
}

// 生成するオブジェクトがあるかを見る
void GameObjectManager::CheckCreateObject()
{
	// 生成するオブジェクトがあれば、オブジェクトリスト内に挿入する
	if (!create_object.empty())
	{
		for (GameObjectBase* obj : create_object)
		{
			// レイヤー情報を基に順番を入れ替える
			int z_layer = obj->GetZLayer();
			std::vector<GameObjectBase*>::iterator itr = game_object.begin();	// オブジェクトリストの先頭
			// リストの末尾になるまで走査する
			for (; itr != game_object.end(); itr++)
			{
				// Zレイヤーが大きい場所に要素を追加する
				// 例 itr->ZLayer{1, 1, 2, 3}、z_layer = 2 の時
				//    itr->ZLayer{1, 1, 2, z_layer, 3}とする
				if (z_layer < (*itr)->GetZLayer())
				{
					break;
				}
			}
			// リストの途中に挿入する
			game_object.insert(itr, obj);
		}
		// 生成リストを解放する
		create_object.clear();
	}
}

// 削除されるオブジェクトがあるかを見る
void GameObjectManager::CheckDesroyObject()
{
	// 削除配列が空でない場合、インゲーム配列のオブジェクトを破棄する
	if (!destroy_object.empty())
	{
		// 削除配列のオブジェクトの数だけ回す
		for (GameObjectBase* obj : destroy_object)
		{
			std::vector<GameObjectBase*>::iterator itr = game_object.begin();	// オブジェクトリストの先頭
			// リストの末尾になるまで走査する
			for (; itr != game_object.end(); itr++)
			{
				// リスト内にあれば、削除する
				if ((*itr) == obj)
				{
					game_object.erase(itr);
					obj->Finalize();
					delete obj;
					break;
				}
			}
		}
		// 忘れずに削除リストをクリア！
		destroy_object.clear();
	}
}

//std::vector<GameObjectBase*>& GameObjectManager::CreateGameObject()
//{
//	return ;
//}

// 削除配列にいるオブジェクトを削除する
void GameObjectManager::DestroyGameObject(GameObjectBase* target)
{
	// ヌルポチェック
	if (target == nullptr)
	{
		return;
	}

	// 削除配列にあれば、追加しない
	for (GameObjectBase* obj : destroy_object)
	{
		if (obj == target)
		{
			return;
		}
	}

	// 削除配列に追加する
	destroy_object.push_back(target);
}

/// <summary>
/// 当たり判定のチェック
/// </summary>
/// <param name="target">例：プレイヤー</param>
/// <param name="partner">例：敵</param>
void GameObjectManager::CheckCollision(GameObjectBase* target, GameObjectBase* partner)
{
	if (target == nullptr || partner == nullptr)
	{
		return;
	}

	// ★参照で受けるように変更！
	const BoxCollision& tc = target->GetCollision();
	const BoxCollision& pc = partner->GetCollision();

	// 当たり判定が有効な対象同士か確認
	if (tc.IsCheckHitTarget(pc.object_type) || pc.IsCheckHitTarget(tc.object_type))
	{
		// 対角線上の頂点座標を求める
		BoxCollision t_copy = tc;
		BoxCollision p_copy = pc;

		t_copy.point[0] += target->GetLocation() - target->GetBoxSize();
		t_copy.point[1] += target->GetLocation() + target->GetBoxSize();
		p_copy.point[0] += partner->GetLocation() - partner->GetBoxSize();
		p_copy.point[1] += partner->GetLocation() + partner->GetBoxSize();

		if (IsCheckCollision(t_copy, p_copy))
		{
			target->OnHitCollision(partner);
			partner->OnHitCollision(target);
		}
	}
}

const Vector2D GameObjectManager::GetScreenOffset() const
{
	return screen_offset;
}

void GameObjectManager::DrawBoss() const
{
	for (GameObjectBase* obj : game_object)
	{
		if (obj && obj->IsBoss())  // ボス判定を追加
		{
			obj->Draw(screen_offset);
		}
	}
}


void GameObjectManager::DrawWithoutBoss() const
{
	for (GameObjectBase* obj : game_object)
	{
		if (obj && !obj->IsBoss())
		{
			obj->Draw(screen_offset);
		}
	}
}

// インゲーム配列にいるオブジェクトを全て削除
void GameObjectManager::DestoryAllObject()
{
	// インゲーム配列が空なら処理を終了する
	if (game_object.empty())
	{
		return;
	}
	// インゲーム配列のオブジェクトを削除する
	for (GameObjectBase* obj : game_object)
	{
		obj->Finalize();
		delete obj;
	}
	// 動的配列（インゲーム配列）の解放
	game_object.clear();
}