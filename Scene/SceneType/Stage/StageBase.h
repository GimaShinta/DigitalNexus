#pragma once

#include "../../../Object/Character/Player/Player.h"
#include "../../../Utility/ProjectConfig.h"
#include "DxLib.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ShakeManager.h"

// StageBase.h
enum class StageID {
    Stage1 = 1,
    Stage2,
    Stage3,
    Stage4,
    Unknown
};

class StageBase {
protected:
    bool is_clear = false;
    bool is_over = false;
    bool is_finished = false;
    int se;
    float stage_timer = 0.0f;
    float scene_timer = 0.0f;
    StageID stage_id;
    class Player* player; // 共有するプレイヤー情報
    bool boss_spawned = false;
    int transparent = 0;
    float trans_timer = 0.0f;

    struct StarParticle {
        Vector2D pos;
        Vector2D velocity;
        float alpha;
        float length;   // 尾の長さ
        float life;
    };

    mutable std::vector<StarParticle> star_particles;

    int font_orbitron;
    int font_warning;


    // 警告ラベル演出用
    enum class WarningLabelState {
        None,
        Expanding,
        Displaying,
        SlideOut,
        Shrinking
    };

    WarningLabelState warning_label_state = WarningLabelState::None;
    float warning_label_timer = 0.0f;
    float warning_label_band_height = 0.0f;

    const float warning_label_max_height = 60.0f;
    const float warning_label_expand_speed = 200.0f;
    const float warning_label_display_duration = 2.5f;

    bool warning_label_shown = false;
    float slide_out_timer = 0.0f;

    Vector2D entry_start_pos = 0.0f;             // 中央あたりから
    Vector2D entry_end_pos = 0.0f; // 通常の定位置
    float player_entry_timer = 0.0f; // タイマーリセット
    bool is_player_entering = true;




public:
    StageBase(Player* player) : player(player) {}
    virtual ~StageBase() {}
    virtual void Initialize() = 0;
    virtual void Update(float delta) = 0;
    virtual void Draw() = 0;
    virtual void Finalize() = 0;
    virtual bool IsFinished() const { return is_finished; }
    virtual void SetFinished() { is_finished = true; }
    virtual bool IsClear() const { return is_clear; }
    virtual bool IsOver() const { return is_over; }
    virtual StageBase* GetNextStage(Player* player) = 0;

public:
    StageID GetStageID() const { return stage_id; }

protected:
    virtual void EnemyAppearance(float delta_second) = 0;       // 敵の出現処理
    // クリア判定
    virtual void UpdateGameStatus(float delta_second) = 0;
    virtual void StageLabel() const = 0;
    virtual void UpdateRabel(float delta_second)
    {
        // ミッションラベル演出（開始3秒間）
        // ステージ開始演出（バンドと文字）
        if (!warning_label_shown || warning_label_state != WarningLabelState::None)
        {
            switch (warning_label_state)
            {
            case WarningLabelState::None:
                warning_label_state = WarningLabelState::Expanding;
                warning_label_timer = 0.0f;
                warning_label_band_height = 0.0f;
                slide_out_timer = 0.0f;
                break;

            case WarningLabelState::Expanding:
                warning_label_band_height += warning_label_expand_speed * delta_second;
                if (warning_label_band_height >= warning_label_max_height)
                {
                    warning_label_band_height = warning_label_max_height;
                    warning_label_state = WarningLabelState::Displaying;
                    warning_label_timer = 0.0f;
                }
                break;

            case WarningLabelState::Displaying:
                warning_label_timer += delta_second;
                if (warning_label_timer >= warning_label_display_duration)
                {
                    warning_label_state = WarningLabelState::SlideOut;
                    slide_out_timer = 0.0f;
                }
                break;

            case WarningLabelState::SlideOut:
                slide_out_timer += delta_second;
                if (slide_out_timer >= 0.5f) // スライドアウト完了
                {
                    warning_label_state = WarningLabelState::Shrinking;
                }
                break;

            case WarningLabelState::Shrinking:
                warning_label_band_height -= warning_label_expand_speed * delta_second;
                if (warning_label_band_height <= 0.0f)
                {
                    warning_label_band_height = 0.0f;
                    warning_label_state = WarningLabelState::None;
                    warning_label_shown = true;
                }
                break;
            }
        }
    }

protected:
    float bg_scroll_speed_layer1 = 100.0f;  // 背面のスクロール速度
    float bg_scroll_speed_layer2 = 200.0f;  // 前面のスクロール速度
    float bg_scroll_offset_layer1 = 0.0f;
    float bg_scroll_offset_layer2 = 0.0f;
    bool is_scroll_enabled = true;

public:
    //スクロール量
    virtual void UpdateBackgroundScroll(float delta)
    {
        if (is_scroll_enabled)
        {
            bg_scroll_offset_layer1 -= bg_scroll_speed_layer1 * delta;
            bg_scroll_offset_layer2 -= bg_scroll_speed_layer2 * delta;

            if (bg_scroll_offset_layer1 >= D_WIN_MAX_Y) bg_scroll_offset_layer1 -= D_WIN_MAX_Y;
            if (bg_scroll_offset_layer2 >= D_WIN_MAX_Y) bg_scroll_offset_layer2 -= D_WIN_MAX_Y;
        }
    }

    virtual void DrawScrollBackground() const = 0; // 各ステージで具体実装

    // StageBase.h の protected 部分に追加
protected:
    // フェード制御
    bool is_fading_in = true;     // 初期状態でフェードイン
    bool is_fading_out = false;   // クリア／オーバー時にフェードアウト開始
    float fade_alpha = 255.0f;    // 0: 完全表示, 255: 完全黒

public:
    // 毎フレーム呼び出すフェード更新
    virtual void UpdateFade(float delta)
    {
        // ステージクリア／オーバーでフェードアウトへ
        if (!is_fading_out && (is_clear || is_over))
        {
            is_fading_out = true;
        }

        // フェードイン中：alpha→0
        if (is_fading_in)
        {
            fade_alpha -= 300.0f * delta;
            if (fade_alpha <= 0.0f)
            {
                fade_alpha = 0.0f;
                is_fading_in = false;
            }
        }
        // フェードアウト中：alpha→255
        else if (is_fading_out)
        {
            fade_alpha += 300.0f * delta;
            if (fade_alpha >= 255.0f)
            {
                fade_alpha = 255.0f;
            }
        }
    }

    // 描画の最後に呼ぶフェード描画
    virtual void DrawFadeOverlay() const
    {
        if (is_fading_in || is_fading_out)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(fade_alpha));
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }

    // グリッチ演出
    bool glitch_started = false;
    bool glitch_done = false;
    float glitch_timer = 0.0f;

    bool entry_effect_playing = true;
    float entry_effect_timer = 0.0f;


    void StartGlitchEffect()
    {
        glitch_started = true;
        glitch_timer = 0.0f;
        glitch_done = false;
    }

    void UpdateGlitchEffect(float delta)
    {
        if (glitch_started && !glitch_done)
        {
            glitch_timer += delta;
            if (glitch_timer >= 3.0f)  // 2秒後に完了
            {
                glitch_done = true;
            }
        }
    }

    void DrawGlitchEffect() const
    {
        if (!glitch_started) return;

        float t = glitch_timer / 2.0f;
        if (t > 1.0f) t = 1.0f;
        int alpha = static_cast<int>(t * 180.0f);  // 濃くなっていく

        for (int i = 0; i < 20; ++i)
        {
            int x = GetRand(D_WIN_MAX_X);
            int y = GetRand(D_WIN_MAX_Y);
            int w = 80 + GetRand(200);
            int h = 5 + GetRand(30);
            int col = GetColor(200 + GetRand(55), 200 + GetRand(55), 255);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(x, y, x + w, y + h, col, TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
};