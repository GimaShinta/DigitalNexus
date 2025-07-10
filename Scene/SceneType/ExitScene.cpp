#include "ExitScene.h"

ExitScene::ExitScene()
{
}

ExitScene::~ExitScene()
{
}

// 初期化処理
void ExitScene::Initialize()
{
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType ExitScene::Update(float delta_second)
{
	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void ExitScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "ExitScene", GetColor(255, 255, 255));
#endif
}

// 終了時処理（使ったインスタンスの削除とか）
void ExitScene::Finalize()
{
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType ExitScene::GetNowSceneType() const
{
	return eSceneType::eExit;
}
