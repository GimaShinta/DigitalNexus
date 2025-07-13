#pragma once
#include "../SceneBase.h"
class StartScene : public SceneBase
{
private:
	enum class Step {
		WaitForInput,
		IntroText,
		GoToTitle,
		DemoMovie,
	};

	Step m_step;
	float m_elapsedTime = 0.0f;
	int m_displayCharCount = 0;
	static const int LINE_COUNT = 2;
	const char* m_introText[LINE_COUNT]; // 2行分の文字列

	int m_fontDigital = -1;
	int m_fontJP = -1;

	int m_seTypeSound = -1;  // 1文字ずつ鳴るSE
	float m_nextCharDelay = 0.0f; // 次の文字までのランダム待機時間

	bool m_demoPlaying = false;
	float m_idleTimer = 0.0f;
	int movie_handle = NULL;

public:
	StartScene();
	virtual ~StartScene();

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

