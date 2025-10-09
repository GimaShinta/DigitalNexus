#pragma once
#include "../StageBase.h"


class Stage1 :   public StageBase
{
private:
	// 各種攻撃・敵キャラクターのポインタ定義
	class Zako* zako = nullptr;               // 雑魚敵1
	class EnemyShot1* e_shot1 = nullptr;      // 敵のショットタイプ1
	class Boss1* boss = nullptr;
	class Enemy4* enemy4 = nullptr;

	//フォント
	int font_orbitron;
	int font_warning;

	//ステージ導入演出：ノイズ
	bool entry_effect_playing = false;
	float entry_effect_timer = 0.0f;

	//デバッグ用：ボス出現スタート
	bool debug_boss_only = false; 

	//ステージの流れ：時間
	float intro_delay_sec = 6.0f;		 //最初のウェーブ開始までの時間
	float wave1_duration_sec = 26.0f;    //ウェーブ１の制限時間
	float miniboss_timeout_sec = 22.0f;  //中ボス強制退場させる時間

	//ウェーブ１の管理用変数
	bool  wave1_started = false;
	bool  wave1_done = false;
	int   wave1_batch = 0;		 // 0..2（3バッチ）
	int   wave1_count = 0;		 //バッチ内の出現数
	int wave1_pattern0[6];		 //列順
	float wave1_next_at = 0.0f;	 // 次の出現予定時刻（stage_timer基準）

	//中ボス管理用変数
	bool  miniboss_spawned = false;
	bool  miniboss_done = false;
	float miniboss_start_t = 0.0f;

	bool  postwave_started = false;
	bool  postwave_done = false;

	// ウェーブ２の流れ：Enemy2
	bool  e2_line_enabled = false;    // LineRise グループ出現を動かすフラグ
	float e2_group_next = 10.2f;	  // 次のグループ開始時刻（stage_timer基準）
	int   e2_group_id = 0;            // 何グループ目か
	float e2_single_next = 11.2f;     // Zako3Like 単発の次時刻（stage_timer基準）
	float e2_group_interval = 3.5f;   // グループ間隔
	float e2_single_interval = 1.6f;  // 単発の間隔
	float postwave_start_at = -1.0f;  //開始ディレイ

	// ウェーブ３の流れ：Enemy5
	bool  e5_started = false;
	bool  e5_done = false;
	int   e5_i_L = 0, e5_i_R = 0;
	float e5_next_L = 0.0f, e5_next_R = 0.0f;

	// ウェーブ４の流れ：Enemy6
	bool  e6_started = false;
	bool  e6_done = false;
	int   e6_lane = 0, e6_count_in_lane = 0;
	float e6_next_at = 0.0f;

	// Boss管理用
	float boss_spawn_at = -1.0f;   // <0: 未予約
	float boss_delay_sec = 9.0f;   // 既存の値を踏襲

	// 左右出現管理
	int spawn_index_left = 0;	//左
	float spawn_delay_timer_left = 1.0f;
	bool spawning_left = false;

	int spawn_index_right = 0;	//右
	float spawn_delay_timer_right = 1.0f;
	bool spawning_right = false;

	// Zako2/Zako7専用のWave管理
	bool zako2_wave_started = false;
	bool zako7_wave_started = false;

private:
	//奥グリッドのスクロール量（低速）
	mutable float scroll_back = 0.0f;  

	//前面グリッドのスクロール量（やや速い）
	mutable float scroll_front = 0.0f;  

private:
	int spawn_index = 0;

	float enemy_spawn_timer = 0.0f;     // 敵出現タイマー
	float spawn_delay_timer = 0.0f;

	bool zako_spawned = false;    // Zakoがすでに出現したかどうか
	bool spawning = false;
	bool spawned_enemy1 = false;  // Enemy1出現フラグ
	bool enemy4_spawned = false;  // 中ボス風ザコ（Enemy4）出現済みフラグ
	bool spawned_enemy1_wave3 = false;

public:
	Stage1(Player* player);     
	~Stage1();                 

	// 基本的なステージ処理群
	void Initialize() override;              // ステージ初期化処理
	void Finalize() override;                // ステージ終了処理
	void Update(float delta_second) override;// 毎フレームの更新処理
	void Draw() override;                    // 描画処理

	StageBase* GetNextStage(Player* player) override; // 次のステージを取得
	bool IsStageLabelActive() const override;

private://ステージ内部処理

	 // 背景のスクロール描画
	void DrawScrollBackground() const override;

	//敵出現処理
	void EnemyAppearance(float delta_second) override;

	//クリア判定
	void UpdateGameStatus(float delta_second) override;

	//ステージラベル
	void StageLabel() const override;

	//プレイヤー登場演出
	void AppearancePlayer(float delta_second);

	//背景エフェクトの更新処理
	void ScrollEffectUpdate(float delta_second); 

	//グリッドの描画
	void DrawFrontGrid() const;                  
};

