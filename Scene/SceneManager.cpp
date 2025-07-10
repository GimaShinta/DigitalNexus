#include "SceneManager.h"
#include "DxLib.h"

// 各シーンのインクルード（必要なシーンは下に追加）
#include "SceneType/StartScene.h"
#include "SceneType/TitleScene.h"
#include "SceneType/GameMainScene.h"
#include "SceneType/RankingScene.h"
#include "SceneType/CreditScene.h"
#include "SceneType/ExitScene.h"

// Singleton継承クラスのインクルード
#include "../Utility/InputManager.h"

// コンストラクタ
SceneManager::SceneManager() :current_scene(nullptr)
{
}

// デストラクタ
SceneManager::~SceneManager()
{
}

// インスタンス（オブジェクトへの参照）の削除
void SceneManager::DeleteInstance()
{
	// 自クラスのポインタ（実体をアドレスの先で保有）
	static SceneManager* instance = nullptr;

	// インスタンスが存在している場合、削除する
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

// 初期化処理
void SceneManager::Initialize()
{
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">1フレーム当たりの時間</param>
/// <returns>終了するかしないか（trueなら終了、falseなら終了しない）</returns>
bool SceneManager::Update(float delta_second)
{
	// 入力情報を取得する
	InputManager* input = Singleton<InputManager>::GetInstance();

	// シーンの更新
	eSceneType next_scene_type = current_scene->Update(delta_second);

	// 現在のシーンタイプが次のシーンタイプと違っているか？
	if (current_scene->GetNowSceneType() != next_scene_type)
	{
		// 違っていたらシーンの切り替え処理
		ChangeScene(next_scene_type);
	}

	// ゲームを終了するか確認する
	if (next_scene_type == eSceneType::eExit ||
		(input->GetButtonUp(XINPUT_BUTTON_BACK)) ||
		input->GetKeyUp(KEY_INPUT_ESCAPE))
	{
		// 振動開始
		StartJoypadVibration(DX_INPUT_PAD1, 1000, 1000, -1);

		// １秒間経過するまで待つ
		WaitTimer(1000);

		//エンドチェック（終了する）
		return true;
	}

	// エンドチェック（終了しない）
	return false;
}

// 描画処理
void SceneManager::Draw()
{
	// current_sceneに入っているシーンタイプの描画処理へ移動
	current_scene->Draw();
}

// 終了時処理（使ったインスタンスの削除とか）
void SceneManager::Finalize()
{
	// シーン情報が生成されていれば、削除する
	if (current_scene != nullptr)
	{
		current_scene->Finalize();
		delete current_scene;
		current_scene = nullptr;
	}
	// インスタンス（クラスへの参照）の削除
	InputManager::DeleteInstance();
}

/// <summary>
/// シーンの切り替え
/// </summary>
/// <param name="scene_type">切り替えたいシーンのタイプ</param>
void SceneManager::ChangeScene(eSceneType type)
{
	// 引数で渡された切り替えたいシーンを作成して、変数に代入
	SceneBase* new_scene = CreateScene(type);

	// エラーチェック
	if (new_scene == nullptr)
	{
		throw("シーンが生成できませんでした\n");
	}

	// 現在シーンの終了処理
	if (current_scene != nullptr)
	{
		current_scene->Finalize();
		delete current_scene;
	}

	// 新しいシーンの初期化
	new_scene->Initialize();

	// 現在のシーンを新しいシーンにする
	current_scene = new_scene;
}

/// <summary>
/// シーン情報の生成
/// </summary>
/// <param name="scene_type">生成したいシーンタイプ</param>
/// <returns>タイトルシーンクラスの情報</returns>
/// <returns>選択シーンクラスの情報</returns>
/// <returns>バトルシーンクラスの情報</returns>
/// <returns>リザルトシーンクラスの情報</returns>
/// <returns>何も返さない</returns>
SceneBase* SceneManager::CreateScene(eSceneType type)
{
	// 引数のシーンタイプによって、生成するシーンを切り替える
	switch (type)
	{
	case eSceneType::eStart:
		return dynamic_cast<SceneBase*>(new StartScene());
	case eSceneType::eTitle:
		return dynamic_cast<SceneBase*>(new TitleScene());
	case eSceneType::eGameMain:
		return dynamic_cast<SceneBase*>(new GameMainScene());
	case eSceneType::eRanking:
		return dynamic_cast<SceneBase*>(new RankingScene());
	case eSceneType::eCredit:
		return dynamic_cast<SceneBase*>(new CreditScene());
	case eSceneType::eExit:
		return dynamic_cast<SceneBase*>(new ExitScene());
		// 上のどれも当てはまらない場合
	default:
		return nullptr;
	}
}