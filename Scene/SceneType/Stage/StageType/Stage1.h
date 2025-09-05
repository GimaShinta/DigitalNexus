#pragma once
#include "../StageBase.h"
class Stage1 :
    public StageBase
{
private:
	// 各種攻撃・敵キャラクターのポインタ定義
	class Zako* zako = nullptr;             // 雑魚敵その1
	class EnemyShot1* e_shot1 = nullptr;      // 敵のショットタイプ1
	class Boss1* boss = nullptr;
	class Enemy4* enemy4 = nullptr;

	float enemy_spawn_timer = 0.0f;     // 敵出現タイマー
	bool zako_spawned = false;         // Zakoがすでに出現したかどうか

	int spawn_index = 0;
	float spawn_delay_timer = 0.0f;
	bool spawning = false;

	bool spawned_enemy1 = false;  // Enemy1出現フラグ

	int font_orbitron;
	int font_warning;

	// 左右出現管理
	int spawn_index_left = 0;
	float spawn_delay_timer_left = 0.0f;
	bool spawning_left = false;

	int spawn_index_right = 0;
	float spawn_delay_timer_right = 0.0f;
	bool spawning_right = false;

	// Zako2/Zako7専用のWave管理
	bool zako2_wave_started = false;
	bool zako7_wave_started = false;

private:
	// 既存のメンバの下に追加
	bool entry_effect_playing = false;
	float entry_effect_timer = 0.0f;

	bool debug_boss_only = false; // ★ デバッグ用: ボスだけ出す
	bool enemy4_spawned = false;  // 中ボス風ザコ（Enemy4）出現済みフラグ
	//bool spawned_enemy1 = false;

	bool spawned_enemy1_wave3 = false;

	// === 追加: Enemy2 連続出現用（Wave2） ===
	bool  e2_line_enabled = false;   // LineRise グループ出現を動かすフラグ
	float e2_group_next = 10.2f;   // 次のグループ開始時刻（stage_timer基準）
	int   e2_group_id = 0;       // 何グループ目か

	float e2_single_next = 11.2f;   // Zako3Like 単発の次時刻（stage_timer基準）
	float e2_group_interval = 3.5f;  // グループ間隔
	float e2_single_interval = 1.6f; // 単発の間隔


	// 既存の private: 節のどこか（他メンバの下でもOK）に追記
// ===== 初心者向けフロー・スケジューラ =====
	float intro_delay_sec = 6.0f;   // 最初の敵は6秒後に出現（早出しバグ封じ）
	float wave1_duration_sec = 18.0f;  // Wave1 の最大持続
	float miniboss_timeout_sec = 16.0f;  // 中ボスの強制退場まで

	bool  wave1_started = false;
	bool  wave1_done = false;

	bool  miniboss_spawned = false;
	bool  miniboss_done = false;
	float miniboss_start_t = 0.0f;

	bool  postwave_started = false;
	bool  postwave_done = false;

	// Wave1 内部
	int   wave1_batch = 0;     // 0..2（3バッチ）
	int   wave1_count = 0;     // バッチ内の出現カウント
	float wave1_next_at = 0.0f;  // 次の出現予定時刻（stage_timer基準）


public:
	// コンストラクタ・デストラクタ
	Stage1(Player* player);     // プレイヤー情報を引数に取るステージ初期化
	~Stage1();                  // デストラクタ

	// 基本的なステージ処理群
	void Initialize() override;              // ステージ初期化処理
	void Finalize() override;                // ステージ終了処理
	void Update(float delta_second) override;       // 毎フレームの更新処理
	void Draw() override;                    // 描画処理
	StageBase* GetNextStage(Player* player) override; // 次のステージを取得

private:
	void DrawScrollBackground() const override; // 背景のスクロール描画
	void EnemyAppearance(float delta_second) override;
	void UpdateGameStatus(float delta_second) override;
	void StageLabel() const override;
	void AppearancePlayer(float delta_second);


};

