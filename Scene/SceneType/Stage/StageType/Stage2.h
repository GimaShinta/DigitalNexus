#pragma once
#include "../StageBase.h"

// 前方宣言
class Zako;
class Boss2;
class EnemyShot1;
class Boss2;        
class Enemy7;

class Stage2 : public StageBase
{
private:
    //敵関連
    Zako* zako = nullptr;
    Enemy7* enemy7_left = nullptr;
    Enemy7* enemy7_right = nullptr;
    EnemyShot1* e_shot1 = nullptr;
    Boss2* boss2 = nullptr;

    bool    boss_spawned = false;        //ボス登場済み
    bool    enemy7_spawned = false;      //Enemy7出現済み
    bool    enemy7_done = false;         //Enemy7撃破
    float   enemy7_start_t = 0.0f;       //Enemy7出現時間
    float   enemy7_timeout_sec = 28.0f;  //Enemy7強制退場時間

    //ステージ進行・UI 
    float stage_timer = 0.0f;  // ステージ経過
    float scene_timer = 0.0f;  // クリア後の遷移待機
    float enemy_spawn_timer = 0.0f;  // 敵出現
    bool  debug_boss_only = false;

    //背景スクロール
    mutable float scroll_back = 0.0f;
    mutable float scroll_front = 0.0f;
    float bg_speed_scale = 1.0f; 

    //ボス登場演出管理
    enum class BossAppearState
    {
        Waiting, Warning, Spawning, Active
    };
    BossAppearState boss_appear_state = BossAppearState::Waiting;
    float           boss_appear_timer = 0.0f;   //演出進行タイマー
    bool            flash_request = false;      // 画面フラッシュ要求
    float           flash_timer = 0.0f;         // フラッシュ減衰

    //フォント
    int font_orbitron;

    //ウェーブ１
    bool  wave1_started = false;
    bool  wave1_done = false;
    int   wave1_group = 0;              //何回目の隊列か
    int   wave1_total_groups = 7;       //隊列の回数
    float wave1_interval = 1.8f;        // 隊列と隊列の間隔（秒）
    int   wave1_count = 0;              // バッチ内のスポーン数
    int   wave1_batch = 0;              // 0..2（3バッチ）
    float wave1_next_at = 0.0f;         // 次の1体を出す時刻

    //ウェーブ２
    bool  wave2_started = false;
    bool  wave2_done = false;
    int   wave2_count = 0;
    float wave2_next_at = 0.0f;

    //ウェーブ３
    bool  wave3_started = false;
    bool  wave3_done = false;
    int   wave3_group = 0;       //0..2（3グループ）
    int   wave3_i = 0;           //グループ内のインデックス
    float wave3_next_at = 0.0f;

    //ステージ演出関連
    bool  entry_effect_playing = true;  //ノイズ演出中
    float entry_effect_timer = 0.0f;    //演出用タイマー
    float delta_draw = 0.0f;            //delta_secondを保存


    bool  stage2boss2_spawned = false;
    bool  zako_spawned = false;
    bool  enemy2_spawned = false;

    // スポーン補助
    bool  spawned_stair_done = false;
    bool  spawned_slash_done = false;
    int   stair_index = 0;
    int   slash_index = 0;
    float stair_timer = 0.0f;
    float slash_timer = 0.0f;

    //背景スクロール
    float bg_scroll_offset_layer1 = 0.0f; // 奥
    float bg_scroll_offset_layer2 = 0.0f; // 手前

public:
    // コンストラクタ・デストラクタ
    Stage2(Player* player);
    ~Stage2();

    // 基本処理
    void Initialize() override;
    void Finalize() override;
    void Update(float delta_second) override;
    void Draw() override;
    StageBase* GetNextStage(Player* player) override;

private:
    // 既存のオーバーライド群
    void DrawScrollBackground() const override;
    void EnemyAppearance(float delta_second) override;
    void UpdateGameStatus(float delta_second) override;
    void StageLabel() const override;

    //内部更新ヘルパ
    void UpdateBackgroundScroll(float delta_second);

private:
    void DrawFrontGrid() const;   //手前グリッド描画


    //全画面フラッシュ（白→減衰）
    void DrawFullScreenFlash();

private:

    void ScrollEffectUpdate(float delta_second);

    // Wave1: Enemy1（整列バッチ）
    bool  s2_wave1_started = false;
    bool s2_wave1_mid = false;   // フェーズBを出したか
    int   s2_wave1_phase = 0;     // 0:左右スウィープ / 1:上レイン
    int   s2_wave1_i = 0;     // そのフェーズ内のインデックス
    float s2_wave1_next = 0.0f;  // 次のスポーン時刻
    bool  s2_wave1_done = false;
    int   s2_wave1_batch = 0;     // 0..2 の3バッチ
    int   s2_wave1_count = 0;     // バッチ内の出現カウント
    float s2_wave1_next_t = 0.0f;  // 次の個体出現時刻

    // Wave2（Enemy2：Snakeのみ）
    bool  s2_wave2_started = false;
    bool  s2_wave2_done = false;
    int   s2_e2_group_id = 0;       // 何グループ目か
    float s2_e2_group_next = 0.0f;    // 次のグループ開始時刻
    float s2_e2_single_next = -1.0f;  // 単発の開始時刻（-1 なら未設定）
    bool  s2_e2_single_used = false;  // そのグループで単発をすでに出したか

    // Wave3: Enemy3（左右交互の列）
    bool  s2_wave3_started = false;
    bool  s2_wave3_done = false;
    int   s2_left_idx = 0;
    int   s2_right_idx = 0;
    float s2_left_delay = 0.0f;
    float s2_right_delay = 0.0f;

    // Wave3 終了時刻を記録して、ボスを少し遅らせて出す
    float s2_wave3_done_time = -1.0f;
    float s2_boss_delay_after_wave3 = 2.5f; // Wave3終了の 2.5秒 後にボス出現

    // Boss2 のスポーン（Warning撤廃で直接スポーン）
    bool  s2_boss_spawned = false;

private:
        // ===== リザルト演出 =====
        float result_timer = 0.0f;
        bool  result_started = false;
        bool  result_finished = false;

        float result_fadeout_timer = 0.0f;
        bool  result_fadeout_started = false;

        // リザルト後のメニュー
        bool  result_menu_active = false;
        float result_menu_open_t = 0.0f;
        float result_menu_text_t = 0.0f;
        float result_menu_blink_t = 0.0f;
        float result_menu_timer = 0.0f;

        // 結果演出用
        float total_score = 0.0f;
        bool  result_displayed = false;
        float clear_wait_timer = 0.0f;
        float glitch_timer = 0.0f;
        bool  glitch_started = false;
        bool  glitch_done = false;
        float post_result_wait_timer = 0.0f;
        bool  result_ended = false;


        // 描画/更新ヘルパ
        void DrawResultMenu();

        float Clamp01(float v)
        {
            if (v < 0.0f) v = 0.0f;
            else if (v > 1.0f) v = 1.0f;
            return v;
        }

        void UpdateResultMenu(float dt);

        // リザルト描画
        void ResultDraw(float delta_second);

};
