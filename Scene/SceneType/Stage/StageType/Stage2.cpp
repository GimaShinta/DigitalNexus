#include "Stage2.h"
#include "Stage3.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Boss/Boss1.h"
#include "../../../../Object/Character/Boss/Boss2.h"
#include "../../../../Utility/EffectManager.h"
#include "../../../../Object/GameObjectManager.h"
#include <cmath>

Stage2::Stage2(Player* player)
    : StageBase(player)
{}

Stage2::~Stage2()
{}

// 初期化処理
void Stage2::Initialize()
{
    // ステージID
    stage_id = StageID::Stage2;

    // タイマー類
    stage_timer = 0.0f;
    enemy_spawn_timer = 0.0f;
    scene_timer = 0.0f;

    // 遷移ノイズ
    entry_effect_playing = true;
    entry_effect_timer = 0.0f;

    // 背景
    bg_scroll_offset_layer1 = 0.0f;
    bg_scroll_offset_layer2 = 0.0f;

    // ラベル
    warning_label_state = WarningLabelState::SlideIn;
    warning_label_timer = 0.0f;
    warning_label_band_height = 0.0f;

    // フォント
    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);

    // ★ ボス登場演出 初期値
    boss_appear_state = BossAppearState::Waiting;
    boss_appear_timer = 0.0f;
    flash_request = false;
    flash_timer = 0.0f;
    bg_speed_scale = 4.0f;
}

// 終了処理
void Stage2::Finalize()
{
    if (font_orbitron != -1)
    {
        DeleteFontToHandle(font_orbitron); font_orbitron = -1;
    }
    if (font_warning != -1)
    {
        DeleteFontToHandle(font_warning);  font_warning = -1;
    }
}

// 更新
void Stage2::Update(float delta_second)
{
    // オブジェクト更新
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Update(delta_second);

    // エフェクト更新
    EffectManager* manager = Singleton<EffectManager>::GetInstance();
    manager->Update(delta_second);

    // ステージ経過
    stage_timer += delta_second;

    // 敵出現（演出フロー込み）
    EnemyAppearance(delta_second);

    // クリア判定
    UpdateGameStatus(delta_second);

    // 背景スクロール更新（加速スケール対応）
    UpdateBackgroundScroll(delta_second);

    // ステージ導入ラベル更新
    UpdateRabel(delta_second);
}

// 描画
void Stage2::Draw()
{
    // 背景
    DrawScrollBackground();

    // オブジェクト
    Singleton<GameObjectManager>::GetInstance()->Draw();

    // エフェクト
    Singleton<EffectManager>::GetInstance()->Draw();

    // ステージ演出ラベル（WARNING含む）
    StageLabel();

    // ★ 全画面フラッシュ（必要時）
    DrawFullScreenFlash();

    // 遷移ノイズ（既存）
    if (entry_effect_playing)
    {
        float t = entry_effect_timer / 1.0f;
        if (t > 1.0f) t = 1.0f;
        int alpha = static_cast<int>((1.0f - t) * 255);

        for (int i = 0; i < 30; ++i)
        {
            int x = GetRand(D_WIN_MAX_X);
            int y = GetRand(D_WIN_MAX_Y);
            int w = 40 + GetRand(100);
            int h = 5 + GetRand(20);
            int col = GetColor(200 + GetRand(55), 200 + GetRand(55), 255);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(x, y, x + w, y + h, col, TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 3);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 255, 255), TRUE); // フラッシュ
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

// 次のステージ
StageBase* Stage2::GetNextStage(Player* player)
{
    return new Stage3(player);
}

// ===== 背景：視差スクロール付きのニューログリッド =====
void Stage2::DrawScrollBackground() const
{
    // 背景色
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);

    // カメラふんわりオフセット
    static Vector2D camera_offset(0, 0);
    static Vector2D camera_target_offset_prev(0, 0);

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
    Vector2D camera_target_offset = camera_target_offset_prev;

    if (player != nullptr)
    {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }
    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    // 視差
    Vector2D layer1_offset = camera_offset * 0.3f; // 奥
    Vector2D layer2_offset = camera_offset * 1.5f; // 手前

    // === 背景グリッド（奥） ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = -grid_size_back; x < D_WIN_MAX_X + grid_size_back; x += grid_size_back)
    {
        int xx = x + (int)layer1_offset.x + (int)bg_scroll_offset_layer1 % grid_size_back;
        DrawLine(xx, 0, xx, D_WIN_MAX_Y, GetColor(30, 60, 120));
    }
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int yy = y + (int)layer1_offset.y + (int)bg_scroll_offset_layer1 % grid_size_back;
        DrawLine(0, yy, D_WIN_MAX_X, yy, GetColor(30, 60, 120));
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // === 前景ライン（手前） ===
    SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
    const int band_h = 4;
    for (int y = -band_h; y < D_WIN_MAX_Y + band_h; y += 28)
    {
        int yy = y + (int)layer2_offset.y + (int)bg_scroll_offset_layer2 % 28;
        DrawBox(0, yy, D_WIN_MAX_X, yy + band_h, GetColor(180, 0, 255), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// 敵の出現（演出フロー込み）
void Stage2::EnemyAppearance(float delta_second)
{
    enemy_spawn_timer += delta_second;
    boss_appear_timer += delta_second;

    switch (boss_appear_state)
    {
        case BossAppearState::Waiting:
            // ★ 5.0秒で警告へ（元はここで即スポーンしていた）:contentReference[oaicite:4]{index=4}
            if (stage_timer >= 5.0f)
            {
                boss_appear_state = BossAppearState::Warning;
                boss_appear_timer = 0.0f;
                bg_speed_scale = 1.2f; // 少し加速
                // TODO: サイレンSEなど
            }
            break;

        case BossAppearState::Warning:
            // 2.0秒ほど警告表示 → スポーンへ
            if (boss_appear_timer >= 2.0f)
            {
                boss_appear_state = BossAppearState::Spawning;
                boss_appear_timer = 0.0f;
                bg_speed_scale = 2.0f; // さらに加速

                if (!stage2boss2_spawned)
                {
                    stage2boss2_spawned = true;
                    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
                    boss2 = objm->CreateObject<Boss2>(Vector2D(640.0f, 240.0f));
                    boss2->Initialize();
                    boss2->SetPlayer(player);
                }

                // 登場瞬間の白フラッシュ
                flash_request = true;
            }
            break;

        case BossAppearState::Spawning:
            // Boss2 のイントロ（ズーム＋パーツ集結）が終わったら戦闘へ
            if (boss2 != nullptr && !boss2->IsIntroActive()) // ★ Boss2側アクセサで判定:contentReference[oaicite:5]{index=5}:contentReference[oaicite:6]{index=6}
            {
                boss_appear_state = BossAppearState::Active;
                boss_appear_timer = 0.0f;
                bg_speed_scale = 1.0f; // 平常に戻す
                flash_request = true;  // 戦闘開始フラッシュ
                // TODO: BGM切り替え
            }
            break;

        case BossAppearState::Active:
            // 通常進行
            break;
    }
}

// クリア・ゲームオーバーの判定
void Stage2::UpdateGameStatus(float delta_second)
{
    // 遷移ノイズ
    if (entry_effect_playing)
    {
        entry_effect_timer += delta_second;
        if (entry_effect_timer >= 1.0f) entry_effect_playing = false;
    }

    // タイムアウトでクリア（任意）
    if (stage_timer >= 120.0f)
    {
        is_clear = true;
    }

    // ボス撃破でクリア
    if ((boss2 != nullptr && boss2->GetIsAlive() == false) && !is_over)
    {
        boss2->SetDestroy();
        is_clear = true;
    }
    if (boss != nullptr && boss->GetIsAlive() == false && !is_over)
    {
        boss->SetDestroy();
        is_clear = true;
    }

    // プレイヤー死亡でゲームオーバー
    if (player != nullptr && player->GetGameOver() && !is_clear)
    {
        is_over = true;
        is_finished = true;
    }

    // ステージ終了処理
    if (is_clear)
    {
        scene_timer += delta_second;
        if (scene_timer >= 2.0f)
        {
            is_finished = true;
        }
    }
}

// ステージ導入ラベル（描画）
void Stage2::StageLabel() const
{
    // ★ WARNING中は専用表示
    if (boss_appear_state == BossAppearState::Warning)
    {
        const float blink = (sinf(GetNowCount() * 0.03f) * 0.5f + 0.5f); // 0?1
        int bandAlpha = (int)(140 + 80 * blink);

        int band_h = 240;
        int band_top = (D_WIN_MAX_Y - band_h) / 2;
        int band_bottom = band_top + band_h;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, bandAlpha);
        DrawBox(0, band_top, D_WIN_MAX_X, band_bottom, GetColor(30, 0, 20), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
        DrawLine(0, band_top, D_WIN_MAX_X, band_top, GetColor(255, 80, 120));
        DrawLine(0, band_bottom, D_WIN_MAX_X, band_bottom, GetColor(255, 80, 120));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        const char* s1 = "!! WARNING !!";
        int w1 = (font_warning != -1) ? GetDrawStringWidthToHandle(s1, (int)strlen(s1), font_warning) : 0;
        int cx = D_WIN_MAX_X / 2;

        if (font_warning != -1)
        {
            SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
            DrawStringToHandle(cx - w1 / 2, band_top + 60, s1, GetColor(255, 120, 160), font_warning);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // グリッチ・ノイズ
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
        for (int i = 0; i < 8; ++i)
        {
            int gx = GetRand(D_WIN_MAX_X - 40);
            int gy = band_top + GetRand(band_h - 6);
            int gw = 40 + GetRand(180);
            int gh = 4 + GetRand(8);
            DrawBox(gx, gy, gx + gw, gy + gh, GetColor(255, 60, 140), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        return;
    }

    // ===== 既存の Stage 2 ラベル（SlideIn/Displaying/SlideOut） =====:contentReference[oaicite:7]{index=7}
    if (warning_label_state != WarningLabelState::None && warning_label_band_height > 1.0f)
    {
        int bandAlpha = 190;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, bandAlpha);

        int band_top = (D_WIN_MAX_Y - (int)warning_label_band_height) / 2;
        int band_bottom = (D_WIN_MAX_Y + (int)warning_label_band_height) / 2;

        // 1) 中央帯
        DrawBox(0, band_top, D_WIN_MAX_X, band_bottom, GetColor(12, 12, 20), TRUE);

        // 2) 薄い罫線
        SetDrawBlendMode(DX_BLENDMODE_ADD, 140);
        DrawLine(0, band_top, D_WIN_MAX_X, band_top, GetColor(60, 150, 255));
        DrawLine(0, band_bottom, D_WIN_MAX_X, band_bottom, GetColor(60, 150, 255));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 3) テキスト（Stage 2 / DIGITAL NEXUS）
        if (font_warning != -1)
        {
            const char* s1 = "STAGE 2";
            const char* s2 = "DIGITAL NEXUS";
            int w1 = GetDrawStringWidthToHandle(s1, (int)strlen(s1), font_warning);
            int w2 = GetDrawStringWidthToHandle(s2, (int)strlen(s2), font_orbitron);
            int cx = D_WIN_MAX_X / 2;

            SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
            DrawStringToHandle(cx - w1 / 2, band_top + 16, s1, GetColor(180, 200, 255), font_warning);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
            DrawStringToHandle(cx - w2 / 2, band_top + 16 + 60, s2, GetColor(200, 220, 255), font_orbitron);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // 4) パルスライン
        int pulseY1 = band_top + (int)(sinf(GetNowCount() * 0.05f) * 3.0f) + 4;
        int pulseY2 = band_bottom + (int)(cosf(GetNowCount() * 0.05f) * 3.0f) - 4;
        SetDrawBlendMode(DX_BLENDMODE_ADD, 100);
        DrawLine(0, pulseY1, D_WIN_MAX_X, pulseY1, GetColor(60, 150, 255));
        DrawLine(0, pulseY2, D_WIN_MAX_X, pulseY2, GetColor(60, 150, 255));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 5) 軽いグリッチ（帯に走る横ノイズ）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
        for (int i = 0; i < 6; ++i)
        {
            int gx = GetRand(D_WIN_MAX_X - 40);
            int gy = band_top + GetRand((int)warning_label_band_height - 6);
            int gw = 40 + GetRand(160);
            int gh = 4 + GetRand(6);
            DrawBox(gx, gy, gx + gw, gy + gh, GetColor(180, 0, 255), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

// ===== 追加：内部更新 =====
void Stage2::UpdateBackgroundScroll(float delta_second)
{
    // 奥レイヤ：緩やかに
    bg_scroll_offset_layer1 += 20.0f * bg_speed_scale * delta_second;
    // 手前レイヤ：速く
    bg_scroll_offset_layer2 += 80.0f * bg_speed_scale * delta_second;

    // 値が大きくなり過ぎないよう適当にラップ
    if (bg_scroll_offset_layer1 > 10000.0f) bg_scroll_offset_layer1 -= 10000.0f;
    if (bg_scroll_offset_layer2 > 10000.0f) bg_scroll_offset_layer2 -= 10000.0f;
}

void Stage2::UpdateRabel(float delta_second)
{
    // ラベルのスライド／表示／退場アニメ（既存）
    const float slideDur = 0.45f;
    const float showDur = 1.10f;

    switch (warning_label_state)
    {
        case WarningLabelState::SlideIn:
            warning_label_timer += delta_second;
            // 0→1 で帯が伸びる
            {
                float t = warning_label_timer / slideDur;
                if (t > 1.0f) t = 1.0f;
                // 0→240px くらいまで伸ばす
                warning_label_band_height = 240.0f * t;
                if (warning_label_timer >= slideDur)
                {
                    warning_label_timer = 0.0f;
                    warning_label_state = WarningLabelState::Displaying;
                }
            }
            break;

        case WarningLabelState::Displaying:
            warning_label_timer += delta_second;
            // 高さは一定
            if (warning_label_timer >= showDur)
            {
                warning_label_timer = 0.0f;
                warning_label_state = WarningLabelState::SlideOut;
            }
            break;

        case WarningLabelState::SlideOut:
            warning_label_timer += delta_second;
            {
                float t = warning_label_timer / slideDur;
                if (t > 1.0f) t = 1.0f;
                // 240→0 に縮む
                warning_label_band_height = 240.0f * (1.0f - t);
                if (warning_label_timer >= slideDur)
                {
                    warning_label_timer = 0.0f;
                    warning_label_state = WarningLabelState::None;
                    warning_label_band_height = 0.0f;
                }
            }
            break;

        case WarningLabelState::None:
        default:
            // 何もしない
            break;
    }
}

// ★ 追加：全画面フラッシュ（白→減衰）
void Stage2::DrawFullScreenFlash()
{
    // フラッシュ要求が出たらタイマ起動
    if (flash_request)
    {
        flash_request = false;
        flash_timer = 0.25f; // 250msの白フラッシュ
    }

    if (flash_timer > 0.0f)
    {
        // ここはフレーム時間を渡せるなら delta を使ってOK
        float decay = 1.0f / 60.0f; // 60fps想定の簡易減衰
        flash_timer -= decay;
        if (flash_timer < 0.0f) flash_timer = 0.0f;

        int alpha = (int)(255.0f * (flash_timer / 0.25f));
        if (alpha > 0)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
}
