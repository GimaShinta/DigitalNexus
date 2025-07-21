#include "Stage2.h"
#include "Stage3.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Boss/Boss1.h"

Stage2::Stage2(Player* player)
    : StageBase(player)
{
}

Stage2::~Stage2()
{
}

// 初期化処理
void Stage2::Initialize()
{
    // ステージIDの設定
    stage_id = StageID::Stage2;

    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);
}

// 終了時処理
void Stage2::Finalize()
{
    if (font_orbitron != -1) {
        DeleteFontToHandle(font_orbitron);
        font_orbitron = -1;
    }
    if (font_warning != -1) {
        DeleteFontToHandle(font_warning);
        font_warning = -1;
    }
}

// 更新処理
void Stage2::Update(float delta_second)
{
    // オブジェクトとエフェクトの更新処理
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Update(delta_second);

    EffectManager* manager = Singleton<EffectManager>::GetInstance();
    manager->Update(delta_second);

    // ステージ管理用タイマー
    stage_timer += delta_second;

    // 敵の出現
    EnemyAppearance(delta_second);

    // クリア判定
    UpdateGameStatus(delta_second);

    // スクロールの更新処理
    UpdateBackgroundScroll(delta_second);

    // 登場時のステージラベルの更新処理
    UpdateRabel(delta_second);
}

// 描画処理
void Stage2::Draw()
{
    // 背景スクロールの描画
    DrawScrollBackground();

    // オブジェクトの描画処理
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Draw();

    // エフェクトの描画処理
    EffectManager* em = Singleton<EffectManager>::GetInstance();
    em->Draw();

    // -------- ステージ演出：Neural Grid --------
    StageLabel();

    // 遷移された瞬間のノイズの描画
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
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 255, 255), TRUE); // フラッシュ効果
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    //// ゲームオーバー時の演出
    //if (is_over)
    //{
    //    SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
    //    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    //    SetFontSize(32);
    //    DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
    //    SetFontSize(16);
    //    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    //}
}

// 次のステージを取得
StageBase* Stage2::GetNextStage(Player* player)
{
    return new Stage3(player);
}

void Stage2::DrawScrollBackground() const
{
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);

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

    // レイヤーごとの視差倍率
    Vector2D layer1_offset = camera_offset * 0.3f; // 奥（背景グリッド）
    Vector2D layer2_offset = camera_offset * 1.5f; // 手前（前景ライン）

    // === 背景グリッドレイヤー（奥） ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
    {
        int draw_x = x - static_cast<int>(layer1_offset.x);
        DrawLine(draw_x, 0, draw_x, D_WIN_MAX_Y, GetColor(0, 100, 255));
    }
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        sy -= static_cast<int>(layer1_offset.y);
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(0, 100, 255));
    }

    // === 前景グリッドレイヤー（手前） ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
    {
        int draw_x = x - static_cast<int>(layer2_offset.x);
        DrawBox(draw_x - 1, 0, draw_x + 1, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
    }
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        sy -= static_cast<int>(layer2_offset.y);
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(180, 0, 255), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// ステージ管理用タイマーを使って敵を生成
void Stage2::EnemyAppearance(float delta_second)
{
    enemy_spawn_timer += delta_second;
    const float spawn_interval = 1.0f;
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    // --- 0～10秒：お山フォーメーション（3レーン、重複抑制） ---
    if (stage_timer < 20.0f)
    {
        static int previous_lane = -1;  // 前回のレーン記録用

        if (enemy_spawn_timer >= spawn_interval)
        {
            enemy_spawn_timer = 0.0f;

            // 前回と違うレーンを選ぶ
            int lane;
            do
            {
                lane = std::rand() % 3;  // 0:左, 1:中央, 2:右
            } while (lane == previous_lane);
            previous_lane = lane;

            float base_x;
            if (lane == 0)      base_x = 400.0f;
            else if (lane == 1) base_x = 640.0f;
            else                base_x = 900.0f;

            float base_y = 0.0f;
            float offset_x = 50.0f;
            float offset_y = 30.0f;

            Enemy1* zako_top = objm->CreateObject<Enemy1>(Vector2D(base_x, base_y - offset_y));
            zako_top->SetPattern(Enemy1Pattern::MoveStraight);
            zako_top->SetPlayer(player);

            Enemy1* zako_left = objm->CreateObject<Enemy1>(Vector2D(base_x - offset_x, base_y + offset_y - 70));
            zako_left->SetPattern(Enemy1Pattern::MoveStraight);
            zako_left->SetPlayer(player);

            Enemy1* zako_right = objm->CreateObject<Enemy1>(Vector2D(base_x + offset_x, base_y + offset_y - 70));
            zako_right->SetPattern(Enemy1Pattern::MoveStraight);
            zako_right->SetPlayer(player);
        }
    }

    // --- 10?15秒：右下がり ＼ の階段状に3体を順番に出現（1回限り） ---
    if (stage_timer >= 10.0f && stage_timer < 15.0f && !spawned_stair_done)
    {
        stair_timer += delta_second;

        const float stair_spawn_interval = 0.6f;  // 出現間隔（秒）

        if (stair_index < 3 && stair_timer >= stair_spawn_interval)
        {
            stair_timer = 0.0f;

            float base_x = 1000.0f;
            float base_y = 40.0f;
            float offset_x = 30.0f;
            float offset_y = 60.0f;

            float x = base_x + stair_index * offset_x;
            float y = base_y + stair_index * offset_y;

            Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, y));
            if (zako != nullptr)
            {
                zako->SetPattern(Enemy1Pattern::LeftMove);
                zako->SetPlayer(player);
            }

            stair_index++;
        }

        if (stair_index >= 3)
        {
            spawned_stair_done = true;
        }
    }

    // --- 15?20秒：右上がり ／ の階段状に3体を順番に出現（1回限り） ---


    if (stage_timer >= 15.0f && stage_timer < 20.0f && !spawned_slash_done)
    {
        slash_timer += delta_second;

        const float slash_spawn_interval = 0.6f;  // 出現間隔（秒）

        if (slash_index < 3 && slash_timer >= slash_spawn_interval)
        {
            slash_timer = 0.0f;

            float base_x = 270.0f;
            float base_y = 200.0f;
            float offset_x = 40.0f;
            float offset_y = -50.0f;

            float x = base_x + slash_index * offset_x;
            float y = base_y + slash_index * offset_y;

            Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, y));
            zako->SetPattern(Enemy1Pattern::RightMove);
            zako->SetPlayer(player);

            slash_index++;
        }

        if (slash_index >= 3)
        {
            spawned_slash_done = true;
        }
    }

    // --- 20～40秒：Zako5を2体だけ1回出現 ---
    if (stage_timer >= 20.0f && stage_timer < 40.0f && !enemy2_spawned)
    {
        enemy_spawn_timer = 0.0f;
        enemy2_spawned = true;

        Enemy2* left = objm->CreateObject<Enemy2>(Vector2D(360.0f, 300.0f));
        left->EnableRectangularLoopMove(false);
        left->SetPlayer(player);

        Enemy2* right = objm->CreateObject<Enemy2>(Vector2D(920.0f, 300.0f));
        right->EnableRectangularLoopMove(false);
        right->SetPlayer(player);
    }

    // --- 40～60秒：お山フォーメーション（ランダム位置） ---
    if (stage_timer >= 42.0f && stage_timer < 55.0f)
    {
        if (enemy_spawn_timer >= spawn_interval)
        {
            enemy_spawn_timer = 0.0f;

            float base_x = 320.0f + std::rand() % 640;
            float base_y = 0.0f;
            float offset_x = 40.0f;
            float offset_y = 30.0f;

            Enemy1* zako_top = objm->CreateObject<Enemy1>(Vector2D(base_x, base_y - offset_y));
            zako_top->SetPattern(Enemy1Pattern::MoveStraight);
            zako_top->SetPlayer(player);

            Enemy1* zako_left = objm->CreateObject<Enemy1>(Vector2D(base_x - offset_x, base_y + offset_y));
            zako_left->SetPattern(Enemy1Pattern::MoveStraight);
            zako_left->SetPlayer(player);

            Enemy1* zako_right = objm->CreateObject<Enemy1>(Vector2D(base_x + offset_x, base_y + offset_y));
            zako_right->SetPattern(Enemy1Pattern::MoveStraight);
            zako_right->SetPlayer(player);
        }
    }

    // --- 60～110秒：ボス出現（1回のみ） ---

    //if (!stage2boss_spawned && stage_timer >= 60.0f)
    //{
    //    stage2boss_spawned = true;

    //    boss = objm->CreateObject<Stage2Boss>(Vector2D(640.0f, -200.0f)); // 高所
    //    boss->Initialize();
    //    boss->SetPlayer(player);
    //    enemy_list.push_back(boss);

    //    // 回転パーツも GameObjectBase* として破棄対象に登録
    //    for (auto& part : boss->GetRotatingParts())
    //    {
    //        extra_destroy_list.push_back(part);
    //    }

    //    phase = Stage2Phase::BossDescending;
    //    is_warning = false;
    //}
}

// クリア判定
void Stage2::UpdateGameStatus(float delta_second)
{
    // 遷移の際のノイズの表示on
    if (entry_effect_playing)
    {
        entry_effect_timer += delta_second;
        if (entry_effect_timer >= 1.0f) // 1秒間表示
        {
            entry_effect_playing = false;
        }
    }

    if (stage_timer >= 40.0f)
    {
        is_clear = true;
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

    // ステージ終了時の動き
    if (is_clear == true)
    {
        // 少し待機したら終了
        scene_timer += delta_second;

        if (scene_timer >= 2.0f)
        {
            is_finished = true;
        }
    }
}

void Stage2::StageLabel() const
{
    if (warning_label_state != WarningLabelState::None && warning_label_band_height > 1.0f)
    {
        int y_top = static_cast<int>(360 - warning_label_band_height);
        int y_bottom = static_cast<int>(360 + warning_label_band_height);

        // パルスライティング（明滅ライン）
        float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f; // 0.0?1.0
        int pulse_alpha = static_cast<int>(pulse * 180);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
        DrawLine(0, y_top, 1280, y_top, GetColor(120, 220, 255));
        DrawLine(0, y_bottom, 1280, y_bottom, GetColor(120, 220, 255));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // グリッチ風のランダムスキャンライン（ちらつき演出）
        for (int i = 0; i < 8; ++i)
        {
            int glitch_y = y_top + rand() % (2 * static_cast<int>(warning_label_band_height));
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (1280 - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
        if (
            warning_label_state == WarningLabelState::Displaying ||
            warning_label_state == WarningLabelState::SlideOut
            )
        {
            int center_x = D_WIN_MAX_X / 2;

            // スライドイン
            float slide_in_t = warning_label_timer / 0.5f;
            if (slide_in_t > 1.0f) slide_in_t = 1.0f;
            float slide_in = 1.0f - powf(1.0f - slide_in_t, 3.0f);

            // スライドアウト（右→左）
            float slide_out_t = slide_out_timer / 0.5f;
            if (slide_out_t > 1.0f) slide_out_t = 1.0f;
            float slide_out = 1.0f + slide_out_t;  // ? 左へ流す補正

            float t = (warning_label_state == WarningLabelState::Displaying) ? slide_in : slide_out;

            int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
            int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

            DrawStringToHandle(stage_name_x, 320,
                "Memory Forge", GetColor(255, 255, 255), font_warning);

            DrawStringToHandle(sub_text_x, 370,
                "Eliminate all hostile units.", GetColor(120, 255, 255), font_orbitron);
        }
    }
}
