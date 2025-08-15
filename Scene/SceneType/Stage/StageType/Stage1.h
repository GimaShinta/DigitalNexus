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
	//bool spawned_enemy1 = false;

	bool spawned_enemy1_wave3 = false;

	// === 追加: Enemy2 連続出現用（Wave2） ===
	bool  e2_line_enabled = false;   // LineRise グループ出現を動かすフラグ
	float e2_group_next = 10.2f;   // 次のグループ開始時刻（stage_timer基準）
	int   e2_group_id = 0;       // 何グループ目か

	float e2_single_next = 11.2f;   // Zako3Like 単発の次時刻（stage_timer基準）
	float e2_group_interval = 3.5f;  // グループ間隔
	float e2_single_interval = 1.6f; // 単発の間隔


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

