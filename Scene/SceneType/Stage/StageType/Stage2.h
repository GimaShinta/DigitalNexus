#pragma once
#include "../StageBase.h"

// 前方宣言
class Zako;
class Boss1;
class EnemyShot1;
class Boss2;

class Stage2 : public StageBase
{
private:
    // ===== 敵関連 =====
    Zako* zako = nullptr;
    Boss1* boss = nullptr;
    EnemyShot1* e_shot1 = nullptr;
    Boss2* boss2 = nullptr;

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

    // ===== ステージ進行・UI =====
    float stage_timer = 0.0f;  // ステージ経過
    float enemy_spawn_timer = 0.0f;  // 敵出現
    float scene_timer = 0.0f;  // クリア後の遷移待機

    // 遷移ノイズ
    bool  entry_effect_playing = true;
    float entry_effect_timer = 0.0f;

    // フォント
    int   font_orbitron = -1;
    int   font_warning = -1;

    // ===== 背景スクロール =====
    float bg_scroll_offset_layer1 = 0.0f; // 奥
    float bg_scroll_offset_layer2 = 0.0f; // 手前

    // ===== ステージ導入ラベル =====
    enum class WarningLabelState
    {
        None, SlideIn, Displaying, SlideOut
    };
    WarningLabelState warning_label_state = WarningLabelState::None;
    float warning_label_timer = 0.0f;
    float warning_label_band_height = 0.0f; // 横帯の高さ（アニメ用）

    // ===== 追加：ボス登場演出の状態 =====
    enum class BossAppearState
    {
        Waiting, Warning, Spawning, Active
    };
    BossAppearState boss_appear_state = BossAppearState::Waiting;
    float           boss_appear_timer = 0.0f;
    bool            flash_request = false; // 画面フラッシュ要求
    float           flash_timer = 0.0f; // フラッシュ減衰
    float           bg_speed_scale = 1.0f; // 背景加速スケール

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

    // 追加：内部更新ヘルパ
    void UpdateBackgroundScroll(float delta_second);
    void UpdateRabel(float delta_second);

    // ★ 追加：全画面フラッシュ（白→減衰）
    void DrawFullScreenFlash();
};
