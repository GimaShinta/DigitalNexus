#include "Stage4.h"
#include "Stage3.h"
#include "../../../../Object/Character/Boss/Boss4.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"
#include "../../../../Utility/ScoreData.h"

Stage4::Stage4(Player* player)
    : StageBase(player)
{
}

Stage4::~Stage4()
{
}

void Stage4::Initialize()
{
    stage_id = StageID::Stage4;

    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);
}

void Stage4::Finalize()
{
    if (font_orbitron != -1) {
        DeleteFontToHandle(font_orbitron);
        font_orbitron = -1;
    }
    if (font_warning != -1) {
        DeleteFontToHandle(font_warning);
        font_warning = -1;
    }

    // ステージ終了時に加算
    if(player)
        ScoreData::GetInstance()->AddScore(static_cast<float>(player->GetLife() * 1000));
}

void Stage4::Update(float delta_second)
{
    // オブジェクトとエフェクトの更新処理
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Update(delta_second);

    EffectManager* manager = Singleton<EffectManager>::GetInstance();
    manager->Update(delta_second);

    // ステージ管理用タイマー
    stage_timer += delta_second;
    delta_draw = delta_second;  // ← delta を保存

    // 敵の出現
    EnemyAppearance(delta_second);

    // クリア判定
    UpdateGameStatus(delta_second);

    // スクロールの更新処理
    UpdateBackgroundScroll(delta_second);

    // 登場時のステージラベルの更新処理
    UpdateRabel(delta_second);
}

void Stage4::Draw()
{
    // 背景スクロールの描画
    DrawScrollBackground(); 

    // オブジェクトの描画処理
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    // エフェクトの描画処理
    EffectManager* em = Singleton<EffectManager>::GetInstance();


    if (draw_animation_first)  // ← このフラグで順序を切り替える
    {
        DrawScrollBackground(); // 背景 & 背面グリッド（奥）

        // グリッドの裏に描画するタイミング：未生成 or 墜落中
        if (boss != nullptr && (!boss->GetGenerate()))
        {
            objm->DrawBoss();
        }

        em->Draw();       // 先にアニメーション

        // 通常戦闘中：生成済み & 墜落していない → グリッドの前に描画
        if (boss != nullptr && boss->GetGenerate())
        {
            objm->DrawBoss();
        }

        // オブジェクト描画の前に必ずブレンドモードを戻す
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        objm->DrawWithoutBoss();      // プレイヤー・ザコなど
    }
    else
    {
        // グリッドの裏に描画するタイミング：未生成 or 墜落中
        if (boss != nullptr && (!boss->GetGenerate()))
        {
            objm->DrawBoss();
        }

        DrawScrollBackground(); // 背景 & 背面グリッド（奥）

        // 通常戦闘中：生成済み & 墜落していない → グリッドの前に描画
        if (boss != nullptr && boss->GetGenerate())
        {
            objm->DrawBoss();
        }
        // オブジェクト描画の前に必ずブレンドモードを戻す
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        objm->DrawWithoutBoss();      // プレイヤー・ザコなど
        em->Draw();       // 後にアニメーション
    }

    // -------- ステージ演出：Neural Grid --------
    StageLabel();

    // クリア時の演出
    if (is_clear)
    {
        if (result_started)
        {
            int fade_alpha = (result_timer * 12.0f < 60.0f) ? static_cast<int>(result_timer * 12.0f) : 60;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, fade_alpha);
            DrawBox((D_WIN_MAX_X / 2) - 350, 0, (D_WIN_MAX_X / 2) + 350, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
            ResultDraw(delta_draw);
        }
    }
    //// ゲームオーバー時の演出
    //else if (is_over)
    //{
    //    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    //    SetFontSize(32);
    //    DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
    //    SetFontSize(16);
    //}
}

// 次のステージを取得
StageBase* Stage4::GetNextStage(Player* player)
{
    return nullptr;
}

// 背景スクロールの描画
void Stage4::DrawScrollBackground() const
{
    static float time = 0.0f;
    time += scroll_timer;

    // === カメラふんわりオフセット（プレイヤー位置に応じて） ===
    static Vector2D camera_offset(0, 0);               // 描画用のふんわりオフセット
    static Vector2D camera_target_offset_prev(0, 0);   // 前回のターゲット位置（死亡後用）

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);

    Vector2D camera_target_offset = camera_target_offset_prev; // デフォルトは前回値を使う

    // プレイヤーが存在していれば追従（位置を更新）
    if (player != nullptr)
    {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }
    else
    {
        // 死亡後も offset_prev を変えない → 固定視点
        camera_target_offset = camera_target_offset_prev;
    }

    // オフセットをなめらかに反映
    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    // 視差オフセット（手前ほど大きく動く）
    Vector2D offset_back = camera_offset * 0.3f; // 背面グリッド（奥）
    Vector2D offset_front = camera_offset * 1.5f; // 前面グリッド（手前）
    Vector2D offset_noise = camera_offset * 1.2f; // ノイズ（最前面）

    // === 背景色（赤みグレー） ===
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(30, 10, 10), TRUE);

    // === 背面レイヤー（奥・細かく・暗め） ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
    {
        int draw_x = x - static_cast<int>(offset_back.x);
        DrawLine(draw_x, 0, draw_x, D_WIN_MAX_Y, GetColor(100, 0, 0));
    }
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        sy -= static_cast<int>(offset_back.y);
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(100, 0, 0));
    }

    // === 前面レイヤー（手前・太く・警告色） ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
    {
        int draw_x = x - static_cast<int>(offset_front.x);
        DrawBox(draw_x - 1, 0, draw_x + 1, D_WIN_MAX_Y, GetColor(255, 40, 40), TRUE);
    }
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        sy -= static_cast<int>(offset_front.y);
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(255, 40, 40), TRUE);
    }

    // === ノイズ演出（最前面・揺れ感） ===
    for (int i = 0; i < 5; ++i)
    {
        if (rand() % 70 == 0)
        {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;

            nx -= static_cast<int>(offset_noise.x);
            ny -= static_cast<int>(offset_noise.y);

            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(255, 100, 50), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// ステージ管理用タイマーを使って敵を生成
void Stage4::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    if (boss4_spawned) return;


    if (stage_timer < 10.0f)
    {
    }
    else
    {
        objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss4>(Vector2D(D_WIN_MAX_X / 2, -100));
        boss->SetPlayer(player);
        boss4_spawned = true;
    }
}

// クリア判定
void Stage4::UpdateGameStatus(float delta_second)
{
    // ボスが倒れたらレイヤーを変更する
    if (boss != nullptr && boss->GetIsCrashing() == true)
    {
        draw_animation_first = true;
    }

    // ボスが倒れたらクリア
    if (boss != nullptr && boss->GetIsAlive() == false && is_over == false)
    {
        boss->SetDestroy();
        is_clear = true;
    }

    // プレイヤーが倒れたらゲームオーバー
    if (player != nullptr && player->GetGameOver() && is_clear == false)
    {
        is_over = true;
        is_finished = true;
    }

    // クリアしたらリザルトを描画
    if (is_clear == true && result_started == false)
    {
        clear_wait_timer += delta_second;
        if (clear_wait_timer >= 5.0f)
        {
            result_started = true;
            result_timer = 0.0f; // スコア演出タイマーリセット
        }
    }
}

void Stage4::ResultDraw(float delta_second)
{
    if (!result_started) return;

    // 秒単位でタイマー進行
    if (!result_fadeout_started)
        result_timer += delta_second;
    else
        result_fadeout_timer += delta_second;

    const int cx = D_WIN_MAX_X / 2;
    const int cy = D_WIN_MAX_Y / 2 - 20;

    // 背景フェード（最大60）
    int fade_alpha = static_cast<int>((result_timer * 12.0f < 60.0f) ? result_timer * 12.0f : 60);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, fade_alpha);
    DrawBox(cx - 350, 0, cx + 350, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // フェード／スライド定数（秒）
    const float fade_duration = 1.0f; // 60フレーム相当
    const int slide_distance = 60;

    // 表示補助関数
    auto GetAlpha = [&](float delay_sec, bool is_fadeout = false) -> int {
        float t = (result_fadeout_started ? result_fadeout_timer : result_timer) - delay_sec;
        if (t < 0.0f) return is_fadeout ? 255 : 0;
        if (t >= fade_duration) return is_fadeout ? 0 : 255;
        return is_fadeout ? static_cast<int>(255 * (1.0f - t / fade_duration))
            : static_cast<int>(255 * t / fade_duration);
        };

    auto GetSlideY = [&](int base_y, float delay_sec, bool is_fadeout = false) -> int {
        float t = (result_fadeout_started ? result_fadeout_timer : result_timer) - delay_sec;
        if (t < 0.0f) return base_y + (is_fadeout ? 0 : slide_distance);
        if (t >= fade_duration) return base_y + (is_fadeout ? slide_distance : 0);
        int offset = static_cast<int>((slide_distance * t) / fade_duration);
        return is_fadeout ? base_y + offset : base_y + slide_distance - offset;
        };

    // スコア集計
    ScoreData* score = Singleton<ScoreData>::GetInstance();
    float base_score = score->GetTotalScore();
    int life_bonus = player->GetLife() * 1000;
    total_score = base_score + life_bonus;

    // 表示ライン設定
    struct ResultLine {
        int delay_frame;      // フレーム単位で定義（後で秒に変換）
        int y_offset;
        std::string label;
        std::string format;
    };

    std::vector<ResultLine> lines = {
        {  30, -80, "TRUE RESULT", "" },
        {  70, -20, "TOTAL SCORE", "TOTAL SCORE : %.0f" },
        { 110,  20, "LIFE BONUS", "LIFE BONUS : %d" },
        { 160,  80, "FINAL SCORE", "FINAL SCORE : %.0f" },
    };

    // 表示位置（左右揃え用）
    const int label_x = cx - 250;
    const int value_x = cx + 250;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        const auto& line = lines[i];

        int delay_frame = result_fadeout_started ? lines.back().delay_frame - line.delay_frame : line.delay_frame;
        float delay_sec = static_cast<float>(delay_frame) / 60.0f;

        int alpha = GetAlpha(delay_sec, result_fadeout_started);
        int y = GetSlideY(cy + line.y_offset, delay_sec, result_fadeout_started);
        int color = GetColor(255, 255, 255);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        if (line.format.empty()) {
            // ラベルだけ描画（"RESULT" など）
            int label_width = GetDrawStringWidth(line.label.c_str(), line.label.size());
            DrawStringToHandle(cx - label_width / 2, y, line.label.c_str(), color, font_orbitron);
        }
        else {
            // ラベルと値を分けて描画
            char value_buf[64];

            if (line.label == "TOTAL SCORE") {
                sprintf_s(value_buf, "%.0f", base_score);
            }
            else if (line.label == "LIFE BONUS") {
                sprintf_s(value_buf, "%d", life_bonus);
            }
            else if (line.label == "FINAL SCORE") {
                sprintf_s(value_buf, "%.0f", total_score);
            }

            // ラベル描画（左寄せ）
            DrawStringToHandle(label_x, y, line.label.c_str(), color, font_orbitron);

            // 数値描画（右寄せ）
            int value_width = GetDrawStringWidth(value_buf, strlen(value_buf));
            DrawStringToHandle(value_x - value_width, y, value_buf, color, font_orbitron);

            // TOTAL SCORE の横線と終了判定
            if (line.label == "FINAL SCORE") {
                int line_y = y - 20;
                DrawLine(cx - 600, line_y, cx + 600, line_y, GetColor(255, 255, 255));

                if (alpha == 255 && !result_fadeout_started) {
                    result_displayed = true;
                }
            }
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 表示後のグリッチ待機
    if (result_displayed && !glitch_started && !glitch_done) {
        post_result_wait_timer += delta_second;
        if (post_result_wait_timer >= 8.0f) {
            glitch_started = true;
            glitch_timer = 0.0f;
        }
    }

    // グリッチ演出
    if (glitch_started && !glitch_done) {
        glitch_timer += delta_second;
        if (glitch_timer > 2.0f) {
            glitch_done = true;
        }
    }

    // フェードアウト開始
    if (glitch_done && !result_fadeout_started) {
        result_fadeout_started = true;
        result_fadeout_timer = 0.0f;
    }

    // フェードアウト終了で完了
    if (result_fadeout_started && !result_ended) {
        float last_delay_sec = static_cast<float>(lines.back().delay_frame) / 60.0f;
        if (result_fadeout_timer >= fade_duration + last_delay_sec) {
            result_ended = true;
            is_finished = true;
            // シーン遷移処理など必要があればここに
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage4::StageLabel() const
{
    if (warning_label_state != WarningLabelState::None && warning_label_band_height > 1.0f)
    {
        int y_top = static_cast<int>(360 - warning_label_band_height);
        int y_bottom = static_cast<int>(360 + warning_label_band_height);

        // パルスライティング（赤系に変更）
        float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f;
        int pulse_alpha = static_cast<int>(pulse * 180);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
        DrawLine(0, y_top, 1280, y_top, GetColor(255, 80, 80));      // ← 赤めに変更
        DrawLine(0, y_bottom, 1280, y_bottom, GetColor(255, 80, 80)); // ← 同上
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // グリッチ風スキャンライン（赤系に変更）
        for (int i = 0; i < 8; ++i)
        {
            int glitch_y = y_top + rand() % (2 * static_cast<int>(warning_label_band_height));
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (1280 - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(255, 100, 100), TRUE); // ← 柔らか赤
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        if (
            warning_label_state == WarningLabelState::Displaying ||
            warning_label_state == WarningLabelState::SlideOut
            )
        {
            int center_x = D_WIN_MAX_X / 2;

            float slide_in_t = warning_label_timer / 0.5f;
            if (slide_in_t > 1.0f) slide_in_t = 1.0f;
            float slide_in = 1.0f - powf(1.0f - slide_in_t, 3.0f);

            float slide_out_t = slide_out_timer / 0.5f;
            if (slide_out_t > 1.0f) slide_out_t = 1.0f;
            float slide_out = 1.0f + slide_out_t;

            float t = (warning_label_state == WarningLabelState::Displaying) ? slide_in : slide_out;

            int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
            int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

            DrawStringToHandle(stage_name_x, 320,
                "The Core Nexus", GetColor(255, 0, 0), font_warning);  // 赤強調のままでOK

            DrawStringToHandle(sub_text_x, 370,
                "Eliminate all hostile units.", GetColor(255, 160, 160), font_orbitron); // ← 赤寄りに変更
        }
    }
}

