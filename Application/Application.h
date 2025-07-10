#pragma once

// インクルード
#include "../Singleton.h"
#include <string>

// マクロ定義
#define	D_SUCCESS		(0)		// 成功
#define	D_FAILURE		(-1)	// 失敗

// プロトタイプ宣言
// (class Tで制限なしで使用)
// ライブラリ処理の管理
class Application : public Singleton<class T>
{
public:
	Application();
	~Application();

	/// <summary>
	/// dxlib初期化処理
	/// </summary>
	void WakeUp();

	/// <summary>
	/// dxlib更新処理
	/// </summary>
	void Run();

	/// <summary>
	/// dxlib終了時処理
	/// </summary>
	void ShutDown();

private:
	/// <summary>
	/// dxlib描画処理
	/// </summary>
	void Graph()const;

public:
	// フレームレートの制御
	void FreamControl();
	//１フレーム当たりの時間を取得
	const float& GetDeltaSecond();
	//エラー内容の書き込み
	int ErrorThrow(std::string error_log);
};

