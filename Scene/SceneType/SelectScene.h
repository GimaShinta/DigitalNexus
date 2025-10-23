#pragma once
#include "../SceneBase.h"
class SelectScene : public SceneBase
{
private:
	// カーソル
	int stage_cursor = 1;
	int check_cursor = 1;
	// ステージを選択したかどうか
	bool stage_check = false;
	// 各ステージの選択画像
	int stage_image[3];

	// 画像調整用
	int image_x = D_WIN_MAX_X - 100;
	int image_y = D_WIN_MAX_Y / 2;
	float image_scale = 1.0f;

public:
	SelectScene();
	virtual ~SelectScene();

	// 初期化処理
	virtual void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delta_second">１フレーム当たりの時間</param>
	/// <returns></returns>
	virtual eSceneType Update(float delta_second) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <returns></returns>
	virtual void Draw() override;

	// 終了時処理（使ったインスタンスの削除とか）
	virtual void Finalize() override;

	// 現在のシーンタイプ（オーバーライド必須）
	virtual eSceneType GetNowSceneType() const override;
};

