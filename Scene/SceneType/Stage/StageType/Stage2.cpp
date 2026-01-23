#include "Stage2.h"
#include "Stage3.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Enemy/Enemy4.h"
#include "../../../../Object/Character/Enemy/Enemy5.h"
#include "../../../../Object/Character/Enemy/Enemy6.h"
#include "../../../../Object/Character/Enemy/Enemy7.h"
#include "../../../../Object/Character/Enemy/Enemy8.h"
#include "../../../../Object/Character/Boss/Boss1.h"
#include "../../../../Object/Character/Boss/Boss2.h"
#include "../../../../Utility/EffectManager.h"
#include "../../../../Object/GameObjectManager.h"
#include "../../../../Utility/InputManager.h"
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

    // フォント
    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);

    // Stage2::Initialize() の末尾あたりに追記
    // ====== 1分ウェーブ初期化 ======
    s2_wave1_started = s2_wave1_done = false;
    s2_wave1_batch = s2_wave1_count = 0;
    s2_wave1_next_t = 0.0f;

    // Wave2
    s2_wave2_started = false;
    s2_wave2_done = false;
    s2_e2_group_id = 0;
    s2_e2_group_next = 0.0f;
    s2_e2_single_next = -1.0f;
    s2_e2_single_used = false;

    s2_wave3_started = s2_wave3_done = false;
    s2_left_idx = s2_right_idx = 0;
    s2_left_delay = s2_right_delay = 0.0f;

    // Waveフラグを完全にリセット
    wave1_started = wave1_done = false;
    wave2_started = wave2_done = false;
    wave3_started = wave3_done = false;

    s2_wave1_started = s2_wave1_done = false;
    s2_wave2_started = s2_wave2_done = false;
    s2_wave3_started = s2_wave3_done = false;

    enemy7_spawned = false;
    enemy7_done = false;
    enemy7_left = nullptr;
    enemy7_right = nullptr;

    boss_spawned = false;
    s2_boss_spawned = false;
    stage2boss2_spawned = false;
    boss2 = nullptr;

    // 念のためタイマー類もリセット
    wave1_next_at = wave2_next_at = wave3_next_at = 0.0f;
    enemy7_start_t = 0.0f;
    s2_wave3_done_time = -1.0f;
    scene_timer = 0.0f;

    s2_wave3_done_time = -1.0f;
    s2_boss_delay_after_wave3 = 4.5f; // 調整可

    s2_boss_spawned = false;

    //Warning 演出は使わないため、ラベル状態もクリア寄りに
    warning_label_state = WarningLabelState::None;  

    //ボス登場演出 初期値
    boss_appear_state = BossAppearState::Waiting;
    boss_appear_timer = 0.0f;
    flash_request = false;
    flash_timer = 0.0f;
    bg_speed_scale = 4.0f;

    SEManager* am = Singleton<SEManager>::GetInstance();
    am->PlaySE(SE_NAME::Kirikae);
    am->ChangeSEVolume(SE_NAME::Kirikae, 80);
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

    ScoreData* score = Singleton<ScoreData>::GetInstance();
    score->Reset();
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

    ScrollEffectUpdate(delta_second);

    // 背景スクロール更新（加速スケール対応）
    UpdateBackgroundScroll(delta_second);

    // ステージ導入ラベル更新
    UpdateRabel(delta_second);

    // Update の最後あたり
    delta_draw = delta_second;

    // UpdateGameStatus のクリア後ブロック差し替え
    if (is_clear == true && result_started == false) {
        clear_wait_timer += delta_second;
        if (clear_wait_timer >= 5.0f) {
            result_started = true;
            result_timer = 0.0f;
            time_stop = true;
            ScoreData* score = Singleton<ScoreData>::GetInstance();
            score->SetTime(2, game_time_hun, game_time_byou, game_time_miri);
        }
    }

    if (result_menu_active) {
        UpdateResultMenu(delta_second);
    }

    // 最終的にフェードアウト完了で:
    if (result_fadeout_started && !result_ended) {
        result_ended = true;
        result_menu_active = true;
        result_menu_timer = 0.0f;
        // auto_return_enabled = false;  // 初期化時 or クラス内初期値
    }

    // （毎フレーム）
    delta_draw = delta_second;
}

// 描画
void Stage2::Draw()
{
    // 背景
    DrawScrollBackground();

    // 手前グリッド
    DrawFrontGrid();

    // オブジェクト
    Singleton<GameObjectManager>::GetInstance()->Draw();

    // エフェクト
    Singleton<EffectManager>::GetInstance()->Draw();

    // 全画面フラッシュ
    DrawFullScreenFlash();

    // 遷移ノイズ
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

    // ステージ演出ラベル（WARNING含む）
    StageLabel();


    // クリア時の演出
    if (is_clear) {
        if (result_started) {
            // これは既存：リザルト描画（背景の黒板＋スコア面）
            int fade_alpha = (result_timer * 12.0f < 60.0f) ? (int)(result_timer * 12.0f) : 60;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, fade_alpha);
            DrawBox((D_WIN_MAX_X / 2) - 350, 0, (D_WIN_MAX_X / 2) + 350, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
            ResultDraw(delta_draw);
        }

        // リザルト終了後のメニュー
        if (result_menu_active) {
            DrawResultMenu();
        }
    }
}

// 次のステージ
StageBase* Stage2::GetNextStage(Player* player)
{
    return new Stage3(player);
}

//背景スクロール
void Stage2::DrawScrollBackground() const
{
    // === カメラふんわりオフセット ===
    static Vector2D camera_offset(0, 0);
    static Vector2D camera_target_offset_prev(0, 0);

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
    Vector2D camera_target_offset = camera_target_offset_prev;

    if (player != nullptr) {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }

    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    // 視差
    Vector2D offset_grid = camera_offset * 0.3f;
    Vector2D offset_stars = camera_offset * 0.6f;
    Vector2D offset_noise = camera_offset * 0.3f;

    // === グリッド描画 ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back) {
        int draw_x = x - static_cast<int>(offset_grid.x);
        DrawLine(draw_x, 0, draw_x, D_WIN_MAX_Y, GetColor(50, 120, 220));
    }
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back) {
        int sy = y - static_cast<int>(scroll_back) % grid_size_back;
        sy -= static_cast<int>(offset_grid.y);
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(50, 120, 220));
    }

    // === 粒子 ===
    for (const auto& p : star_particles) {
        int a = static_cast<int>(p.alpha);
        if (a <= 0) continue;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

        int px = static_cast<int>(p.pos.x - offset_stars.x);
        int py = static_cast<int>(p.pos.y - offset_stars.y);

        DrawLine(px, py, px, py + static_cast<int>(p.length), GetColor(100, 200, 255));
    }

    // === ノイズエフェクト ===
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    for (int i = 0; i < 5; ++i) {
        if (rand() % 60 == 0) {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;
            nx -= static_cast<int>(offset_noise.x);
            ny -= static_cast<int>(offset_noise.y);
            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(80, 180, 255), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

//背景エフェクト
void Stage2::ScrollEffectUpdate(float delta_second)
{
    //スクロールスピード
    scroll_back -= delta_second * 110.0f;  
    scroll_front -= delta_second * 280.0f;  

    for (auto& p : star_particles) {
        p.pos.y += p.velocity.y * delta_second * 0.6f; 
        p.alpha -= delta_second * 22.0f;               
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p) { return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0); }),
        star_particles.end());

    if (star_particles.size() < 60) {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 60.0f + GetRand(30));
        p.alpha = 100.0f + GetRand(100);
        p.length = 10.0f + GetRand(10);
        p.life = 2.0f + GetRand(100) / 50.0f;
        star_particles.push_back(p);
    }
}

//敵出現
void Stage2::EnemyAppearance(float dt)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    // デバッグ：ボスのみ
    if (debug_boss_only && !boss_spawned) {
        const float CX = D_WIN_MAX_X * 0.5f;
        boss2 = objm->CreateObject<Boss2>(Vector2D(CX, 200.0f));
        if (boss2) boss2->SetPlayer(player);
        boss_spawned = true;
        return;
    }
    if (debug_boss_only) return;

    const float CX = D_WIN_MAX_X * 0.5f;

    // =========================
    //  ウェーブ１
    // =========================
    if (!wave1_started) {
        wave1_started = true;
        wave1_done = false;
        wave1_group = 0;                 
        wave1_next_at = stage_timer + 1.6f;  //開幕を少し遅らせる
    }

    if (!wave1_done) {
        if (stage_timer >= wave1_next_at) {
            const float W = (float)D_WIN_MAX_X;
            const float H = (float)D_WIN_MAX_Y;

            // 三機フォーメーションの横オフセット
            constexpr float SPACING_X = 72.0f;  
            constexpr float START_Y = -80.0f;
            constexpr float TARGET_Y = 170.0f;
            constexpr float CENTER_Y = TARGET_Y + 18.0f;
            constexpr float APPEAR_T = 1.25f;

            //レーン設計
            const float formation_half = SPACING_X;   // 左右 ±SPACING_X に置く
            const float margin = 15.0f;
            const float left = formation_half + margin;
            const float right = W - (formation_half + margin);

            //レーンの展開幅
            const float LANE_SPAN_SCALE = 0.30f; 
            const float mid = (left + right) * 0.5f;
            const float half_span = (right - left) * 0.5f * LANE_SPAN_SCALE;
            const float leftN = mid - half_span;
            const float rightN = mid + half_span;

            // レーン数
            constexpr int LANE_N = 5;

            // レーン中心座標
            auto laneCenter = [&](int laneIdx)->float {
                if (LANE_N <= 1) return (leftN + rightN) * 0.5f;
                return leftN + (rightN - leftN) * (float)laneIdx / (float)(LANE_N - 1);
                };

            //並び順（中央→左端→右端→左中→右中）
            auto laneOrder = [&](int k)->int {
                switch (k % LANE_N) {
                case 0: return 2; // center
                case 1: return 0; // far left
                case 2: return 4; // far right
                case 3: return 1; // mid left
                default:return 3; // mid right
                }
                };

            //今回出すレーン
            const int   laneIdx = laneOrder(wave1_group);
            const float CX = laneCenter(laneIdx);

            // 三機の相対配置（中央だけ少し前）
            struct Slot { float ox; float ty; };
            const Slot slots[3] = {
                { -SPACING_X, TARGET_Y },
                {  0.0f,      CENTER_Y },
                { +SPACING_X, TARGET_Y },
            };

            // スポーン（三機）
            for (int i = 0; i < 3; ++i) {
                Vector2D s(CX + slots[i].ox, START_Y);
                Vector2D t(CX + slots[i].ox, slots[i].ty);
                if (auto* e = objm->CreateObject<Enemy8>(s)) {
                    e->SetAppearParams(s, t, APPEAR_T);
                    e->SetPlayer(player);
                }
            }

            // 次の隊列を予約 or 完了
            wave1_group++;
            if (wave1_group < wave1_total_groups) {
                wave1_next_at += wave1_interval;  //間隔をあけてもう一度
            }
            else {
                wave1_done = true;                //Wave1 完了、次へ進める
            }
        }

        // Wave1 が続いている間はここで止める
        if (!wave1_done) return;
    }


    // =========================
    // ウェーブ２
    // =========================
    if (!wave2_started) {
        wave2_started = true;
        wave2_count   = 0;
        wave2_next_at = stage_timer + 0.8f;
    }

    if (!wave2_done) {
        if (stage_timer >= wave2_next_at) {
            // 12体：左上から右下へ、開始位置に少しバラつき
            constexpr int   COUNT     = 12;
            constexpr float APPEAR_T  = 0.9f;
            constexpr float DX        = 70.0f; // 右へ
            constexpr float DY        = 70.0f; // 下へ

            float sx = 120.0f + (wave2_count % 4) * 60.0f; 
            float sy = -60.0f  - (wave2_count / 4) * 40.0f; 

            Vector2D start_pos(sx, sy);
            Vector2D target_pos(sx + DX * 6.0f, sy + DY * 6.0f); // 斜め移動

            auto e = objm->CreateObject<Enemy1>(start_pos);
            if (e) {
                e->SetAppearParams(start_pos, target_pos, APPEAR_T);
                e->SetPlayer(player);
            }

            wave2_count++;
            if (wave2_count < COUNT) {
                wave2_next_at = stage_timer + 0.20f;
            } else {
                wave2_done = true;
            }
        }
        if (!wave2_done) return;
    }
    //ウェーブ２終了：Enemy7出現
    if (wave2_done && !enemy7_done)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
        const float CX = D_WIN_MAX_X * 0.5f;
        const float W = (float)D_WIN_MAX_X;
        const float H = (float)D_WIN_MAX_Y;

        if (!enemy7_spawned) {
            enemy7_spawned = true;
            enemy7_start_t = stage_timer;

            // 左右の徘徊範囲
            RectF leftArea{ 40.0f,   H * 0.20f,  CX - 24.0f, H * 0.38f };
            RectF rightArea{ CX + 24.0f, H * 0.20f,  W - 40.0f, H * 0.38f };

            // 左：やや左上から入場
            enemy7_left = objm->CreateObject<Enemy7>(Vector2D(CX - 220.0f, -220.0f));
            if (enemy7_left) {
                enemy7_left->SetPlayer(player);
                enemy7_left->SetWanderBounds(leftArea);
                enemy7_left->SetLabel("LEFT");
            }

            // 右：やや右上から入場
            enemy7_right = objm->CreateObject<Enemy7>(Vector2D(CX + 220.0f, -220.0f));
            if (enemy7_right) {
                enemy7_right->SetPlayer(player);
                enemy7_right->SetWanderBounds(rightArea);
                enemy7_right->SetLabel("RIGHT");
            }
        }

        // 2体とも終了したら先へ
        bool left_done = (!enemy7_left) || enemy7_left->IsCleared();
        bool right_done = (!enemy7_right) || enemy7_right->IsCleared();

        if (!left_done && (stage_timer - enemy7_start_t) >= enemy7_timeout_sec) {
            enemy7_left->ForceRetreat(); 
        }
        if (!right_done && (stage_timer - enemy7_start_t) >= enemy7_timeout_sec) {
            enemy7_right->ForceRetreat(); 
        }

        if (left_done && right_done) {
            enemy7_done = true;
        }
        else {
            //中ボスが生きている間はここで止める
            return;
        }
    }
    // =========================
    // ウェーブ３：中央から小集団 × 3
    // =========================
    if (!wave3_started) {
        wave3_started = true;
        wave3_group   = 0;
        wave3_i       = 0;
        wave3_next_at = stage_timer + 1.0f;
    }

    if (!wave3_done) {
        if (stage_timer >= wave3_next_at) {
            // 1グループ = 5体：中央あたりから上下に軽く散る
            constexpr int   PER_GROUP = 5;
            constexpr float APPEAR_T  = 0.8f;
            const float base_y = 240.0f + wave3_group * 30.0f;

            float offset_x[PER_GROUP] = { -80, -40, 0, 40, 80 };
            float offset_y[PER_GROUP] = { -10,  10, 0, -10, 10 };

            Vector2D start_pos(CX, -70.0f);
            Vector2D target_pos(CX + offset_x[wave3_i], base_y + offset_y[wave3_i]);

            auto e = objm->CreateObject<Enemy1>(start_pos);
            if (e) {
                e->SetAppearParams(start_pos, target_pos, APPEAR_T);
                e->SetPlayer(player);
            }

            wave3_i++;
            if (wave3_i < PER_GROUP) {
                wave3_next_at = stage_timer + 0.22f;
            } else {
                wave3_group++;
                wave3_i = 0;
                if (wave3_group < 3) {
                    wave3_next_at = stage_timer + 0.8f;  // 小休止
                } else {
                    wave3_done = true;
                }
            }
        }
        if (!wave3_done) return;
    }

    // =========================
    // Boss2 召喚（Warning省略して即出現）
    // =========================
    if (!boss_spawned) {
        boss_spawned = true;
        boss2 = objm->CreateObject<Boss2>(Vector2D(CX, 220.0f));
        if (boss2) boss2->SetPlayer(player);
    }
}

// クリア・ゲームオーバーの判定
void Stage2::UpdateGameStatus(float delta_second)
{
    // 遷移時のノイズ演出
    if (entry_effect_playing)
    {
        entry_effect_timer += delta_second;
        if (entry_effect_timer >= 1.0f)
        {
            entry_effect_playing = false;
        }
    }

    // ボスが倒れたらクリア
    if (boss2 != nullptr && boss2->GetIsAlive() == false && is_over == false)
    {
        boss2->SetDestroy();
        is_clear = true;
        // ノイズ演出を開始
        entry_effect_playing = true;
        entry_effect_timer = 0.0f;
    }

    // プレイヤーが倒れたらゲームオーバー
    if (player != nullptr && player->GetGameOver() && is_clear == false)
    {
        is_over = true;
        is_finished = true;
    }

    //クリア後の待機状態
    if (is_clear == true && result_started == false) {
        clear_wait_timer += delta_second;
        if (clear_wait_timer >= 5.0f) {
            result_started = true;
            result_timer = 0.0f;
            time_stop = true;
        }
    }
}

// ステージ導入ラベル（描画）
void Stage2::StageLabel() const
{
    if (warning_label_state != WarningLabelState::None && warning_label_band_height > 1.0f)
    {
        const int center_x = D_WIN_MAX_X / 2;
        const int center_y = D_WIN_MAX_Y / 2;

        int y_top = static_cast<int>(center_y - warning_label_band_height);
        int y_bottom = static_cast<int>(center_y + warning_label_band_height);

        // パルスライティング（明滅ライン）? 暗めのネオンブルー
        float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f;
        int pulse_alpha = static_cast<int>(pulse * 180);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
        DrawLine(0, y_top, D_WIN_MAX_X, y_top, GetColor(60, 120, 255)); // 暗青
        DrawLine(0, y_bottom, D_WIN_MAX_X, y_bottom, GetColor(60, 120, 255)); // 暗青
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // グリッチ風のランダムスキャンライン（暗めブルー系）
        for (int i = 0; i < 8; ++i)
        {
            int glitch_y = y_top + rand() % (2 * static_cast<int>(warning_label_band_height));
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (D_WIN_MAX_X - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(100, 160, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // ラベルテキスト
        if (
            warning_label_state == WarningLabelState::Displaying ||
            warning_label_state == WarningLabelState::SlideOut
            )
        {
            float slide_in_t = warning_label_timer / 0.5f;
            if (slide_in_t > 1.0f) slide_in_t = 1.0f;
            float slide_in = 1.0f - powf(1.0f - slide_in_t, 3.0f);

            float slide_out_t = slide_out_timer / 0.5f;
            if (slide_out_t > 1.0f) slide_out_t = 1.0f;
            float slide_out = 1.0f + slide_out_t;

            float t = (warning_label_state == WarningLabelState::Displaying) ? slide_in : slide_out;

            int stage_name_x = static_cast<int>((1.0f - t) * D_WIN_MAX_X + t * (center_x - 150));
            int sub_text_x = static_cast<int>((1.0f - t) * D_WIN_MAX_X + t * (center_x - 150));

            // メインタイトル
            DrawStringToHandle(stage_name_x, center_y - 40,
                "Memory Forge", GetColor(255, 255, 255), font_warning);

            // サブテキスト（暗い青寄り）
            DrawStringToHandle(sub_text_x, center_y + 10,
                "Eliminate all hostile units.", GetColor(120, 160, 255), font_orbitron);
        }
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

// 全画面フラッシュ（白→減衰）
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
        float decay = 1.0f / 60.0f; 
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

void Stage2::DrawFrontGrid() const
{
    static Vector2D camera_offset(0, 0);
    static Vector2D camera_target_offset_prev(0, 0);

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
    Vector2D camera_target_offset = camera_target_offset_prev;

    if (player != nullptr) {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }
    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    Vector2D offset_front = camera_offset * 1.5f;

    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

    // 太い縦線
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front) {
        int draw_x = x - static_cast<int>(offset_front.x);
        DrawBox(draw_x - 1, 0, draw_x + 1, D_WIN_MAX_Y, GetColor(80, 160, 255), TRUE);
    }

    // 太い横線
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front) {
        int sy = y - static_cast<int>(scroll_front) % grid_size_front;
        sy -= static_cast<int>(offset_front.y);
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(80, 160, 255), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

void Stage2::UpdateResultMenu(float dt)
{
    result_menu_timer += dt;
    result_menu_blink_t += dt;

    const float OPEN_SEC = 0.40f;
    if (result_menu_open_t < 1.0f)
    {
        result_menu_open_t += dt / OPEN_SEC;
        if (result_menu_open_t > 1.0f) result_menu_open_t = 1.0f;
    }

    const float TEXT_START = 0.70f;
    const float TEXT_FADE_SEC = 0.30f;
    if (result_menu_open_t >= TEXT_START && result_menu_text_t < 1.0f)
    {
        result_menu_text_t += dt / TEXT_FADE_SEC;
        if (result_menu_text_t > 1.0f) result_menu_text_t = 1.0f;
    }

    // 決定（A or Space）
    InputManager* input = Singleton<InputManager>::GetInstance();
    if (input->GetButtonDown(XINPUT_BUTTON_A) || input->GetKeyDown(KEY_INPUT_SPACE))
    {
        is_finished = true;   // ← セレクトへ戻る
    }
}

// 横伸びのアウトバック（少し勢いを付ける）
static float EaseOutBack(float t, float s = 1.70158f) {
    float u = t - 1.0f;
    return 1.0f + (u * u * ((s + 1.0f) * u + s));
}

void Stage2::DrawResultMenu()
{
    const int cx = D_WIN_MAX_X / 2;
    const int cy = D_WIN_MAX_Y - 120; // 画面下寄り

    // パネル最大サイズ
    const int PANEL_W_FULL = 640;
    const int PANEL_H = 140;
    const int PANEL_W_MIN = 40;

    // 背景のうっすら暗転
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 横に伸びる幅（EaseOutBack）
    float w01 = EaseOutBack(Clamp01(result_menu_open_t));
    int panel_w = PANEL_W_MIN + (int)((PANEL_W_FULL - PANEL_W_MIN) * w01);

    int x0 = cx - panel_w / 2;
    int y0 = cy - PANEL_H / 2;
    int x1 = cx + panel_w / 2;
    int y1 = cy + PANEL_H / 2;

    // パネル本体（内側→外枠）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);
    DrawBox(x0, y0, x1, y1, GetColor(18, 22, 46), TRUE); // 濃紺
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
    DrawBox(x0 - 6, y0 - 6, x1 + 6, y1 + 6, GetColor(40, 70, 160), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 文字フェード（RETURN / Press A）
    float t01 = EaseOutBack(Clamp01(result_menu_open_t));
    int alpha_text = (int)(255.0f * t01);

    // タイトル
    const char* TITLE = "RETURN TO STAGE SELECT";
    int tw = GetDrawStringWidth(TITLE, (int)strlen(TITLE));
    int tx = cx - tw / 2;
    int ty = cy - 30;

    // 影
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(alpha_text * 0.5f));
    DrawStringToHandle(tx + 2, ty + 2, TITLE, GetColor(0, 0, 0), font_orbitron);

    // 本体
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_text);
    DrawStringToHandle(tx, ty, TITLE, GetColor(230, 240, 255), font_orbitron);

    // ヒント（Press A）? フェード＋点滅
    float blink = 0.7f + 0.3f * sinf(result_menu_blink_t * 6.0f);
    int alpha_press = (int)(alpha_text * blink);

    const char* HINT = "Press A";
    int hw = GetDrawStringWidth(HINT, (int)strlen(HINT));
    int hx = cx - hw / 2;
    int hy = cy + 24;

    // 影
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(alpha_press * 0.45f));
    DrawStringToHandle(hx + 1, hy + 1, HINT, GetColor(0, 0, 0), font_orbitron);

    // 本体（ネオンシアン寄り）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_press);
    DrawStringToHandle(hx, hy, HINT, GetColor(120, 240, 255), font_orbitron);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2::ResultDraw(float delta_second)
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

    // 経過時間を秒に換算
    float total_seconds = game_time_hun * 60.0f + game_time_byou + game_time_miri / 1000.0f;

    // タイムボーナス計算
    const int max_bonus = 60000;   // 最大ボーナス 60,000
    const int decay_per_sec = 200; // 1秒ごとの減点 200
    int time_bonus = static_cast<int>(max_bonus - total_seconds * decay_per_sec);
    if (time_bonus < 0) time_bonus = 0;

    total_score = base_score + life_bonus + time_bonus;

    score->SetStageScore(2, total_score);

    // 表示ライン設定
    struct ResultLine {
        int delay_frame;      // フレーム単位で定義（後で秒に変換）
        int y_offset;
        std::string label;
        std::string format;
    };

    std::vector<ResultLine> lines = {
        {  30, -100, "RESULT", "" },
        {  70,  -40, "TOTAL SCORE", "TOTAL SCORE : %.0f" },
        { 110,    0, "LIFE BONUS", "LIFE BONUS : %d" },
        { 150,   40, "TIME BONUS", "TIME BONUS : %d" },
        { 190,  100, "FINAL SCORE", "FINAL SCORE : %.0f" },
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
            else if (line.label == "TIME BONUS") {
                sprintf_s(value_buf, "%d", time_bonus);
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

    //SetFontSize(16);
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

    // （フェードアウト完了のところ）
    if (result_fadeout_started && !result_ended) {
        float last_delay_sec = static_cast<float>(lines.back().delay_frame) / 60.0f;
        if (result_fadeout_timer >= fade_duration + last_delay_sec)
        {
            result_ended = true;
            result_menu_timer = 0.0f;
            result_menu_open_t = 0.0f;
            result_menu_text_t = 0.0f;
            result_menu_blink_t = 0.0f;
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}