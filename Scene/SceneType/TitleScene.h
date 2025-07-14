#pragma once
#include "../SceneBase.h"
#include <vector>
class TitleScene : public SceneBase
{
private:
	int m_selectedIndex = 0; // 選択中のインデックス
	int m_menuFontHandle = 0;

	int m_logoHandle = 0;
	float m_logoScale = 0.0f; // アニメ用スケール
	int m_logoAlpha = 0;   // アニメ用透明度

	//ロゴの座標調整変数
	int m_logoX = 0;
	int m_logoY = 0;
	int m_logoW = 0;
	int m_logoH = 0;

	float m_logoAppearTimer = 0.0f;
	bool m_logoAppeared = false;

	int logo_se = NULL;
	int bgm = NULL;
	int cursor_se = NULL;
	int tap_se = NULL;

	int drawY = 0;
	float m_cursorY = 0.0f; // カーソルY座標



	//選択時状態フラグ
	bool m_startTransitioning = false;  // 出撃演出中フラグ
	float m_transitionTimer = 0.0f;     // 出撃演出タイマー


	float m_menuScale = 1.0f;
	float m_menuAlpha = 255.0f;


	//パーティクルの演出用
	struct Particle {
		float x, y, vy;
		int alpha;
	};
	std::vector<Particle> m_particles;

	float exit_scene_timer = 0.0f;
	bool exit_scene = false;
	float exit_trans_timer = 0.0f;
	int exit_trans = 0;
	int text_alpha = 0;     // 0?255 で文字の明るさ
	bool on_text = false;
public:
	TitleScene();
	virtual ~TitleScene();

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
	//描画系関数
	void DrawBackgroundGrid();	//背景
	void DrawLogo();			//ロゴ
	void DrawParticles();		//パーティクル
	void DrawMenu();			//メニュー

};

