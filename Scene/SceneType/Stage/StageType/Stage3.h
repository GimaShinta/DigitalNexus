#pragma once
#include "../StageBase.h"
class Stage3 :
	public StageBase
{
private:
	// 各種攻撃・敵キャラクターのポインタ定義
	class Zako* zako = nullptr;             // 雑魚敵その1
	class Boss3* boss = nullptr;
	class EnemyShot1* e_shot1 = nullptr;      // 敵のショットタイプ1

	float enemy_spawn_timer = 0.0f;     // 敵出現タイマー
	float enemy3_spawn_timer = 0.0f;
	bool zako_spawned = false;         // Zakoがすでに出現したかどうか
	bool boss_spawned = false;

	// 左出現管理
	int spawn_index_left = 0;
	float spawn_delay_timer_left = 0.0f;
	bool spawning_zako_left = false;

	// 右出現管理
	int spawn_index_right = 0;
	float spawn_delay_timer_right = 0.0f;
	bool spawning_zako_right = false;

	bool is_enemy3_group_spawned = false;
	bool draw_animation_first = false;  // デフォルトはfalse

	//グリッド
	mutable float scroll_back = 0.0f;
	mutable float scroll_front = 0.0f;


	float result_timer = 0.0f;
	float total_score = 0.0f;
	bool result_displayed = false;

	float clear_wait_timer = 0.0f;
	bool result_started = false;

	float glitch_timer = 0.0f;
	bool glitch_started = false;
	bool glitch_done = false;
	float post_result_wait_timer = 0.0f;  // ←これを追加
	float delta_draw = 0.0f;

	bool result_fadeout_started = false;
	float result_fadeout_timer = 0.0f;
	bool result_ended = false;

public:
	// コンストラクタ・デストラクタ
	Stage3(Player* player);     // プレイヤー情報を引数に取るステージ初期化
	~Stage3();                  // デストラクタ

	// 基本的なステージ処理群
	void Initialize() override;              // ステージ初期化処理
	void Finalize() override;                // ステージ終了処理
	void Update(float delta_second) override;       // 毎フレームの更新処理
	void Draw() override;                    // 描画処理
	StageBase* GetNextStage(Player* player) override; // 次のステージを取得

private:
	void DrawScrollBackground() const override; // 背景のスクロール描画
	void ScrollEffectUpdate(float delta_second);
	void DrawFrontGrid() const;
	void EnemyAppearance(float delta_second) override;
	void UpdateGameStatus(float delta_second) override;
	void ResultDraw(float delta_second);

	template <typename T>
	T my_max(const T& a, const T& b)
	{
		return (a > b) ? a : b;
	}
};

