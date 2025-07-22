#pragma once
#include "../SceneBase.h"
#include "../../Utility/ScoreData.h"

class RankingScene : public SceneBase
{
private:
	std::vector<ScoreEntry> entries;
	float total_score = 0.0f;
	int wait_timer = 0;
	int start_time = 0;
	bool confirm_reset = false;
	int cursor_index = 0;  // カーソル位置の追加

	bool reset_command[4] = { false };

public:
	RankingScene();
	virtual ~RankingScene();

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

private:
	float Max(float a, float b)
	{
		return (a > b) ? a : b;
	}

	bool LoadRankingFromFile();
	void SaveRankingToFile();
	void ResetRankingFile();
	std::string GetTodayString();
	void DrawNeonText(int x, int y, const char* text, int baseColor, int glowColor, int glowPulse);
};

