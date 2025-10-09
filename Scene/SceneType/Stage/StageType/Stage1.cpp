#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Enemy/Enemy4.h"
#include "../../../../Object/Character/Enemy/Enemy5.h"
#include "../../../../Object/Character/Enemy/Enemy6.h"
#include "../../../../Object/Character/Boss/Boss1.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage1::Stage1(Player* player): StageBase(player),
font_orbitron(NULL),font_warning(NULL),
wave1_pattern0(){}

Stage1::~Stage1(){}

/// <summary>
/// 初期化処理
/// </summary>
void Stage1::Initialize()
{
    // ステージIDの設定
	stage_id = StageID::Stage1;

    //フォント設定
    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);

    //プレイヤーを登場させる
    if (player)
    {
        entry_start_pos = Vector2D(D_WIN_MAX_X / 2, 360);             // 中央あたりから
        entry_end_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y - 120); // 通常の定位置

        player_entry_timer = 0.0f; // タイマーリセット

        player->SetMobility(false);           // 操作禁止
        player->SetLocation(entry_start_pos); // 最初の位置を設定
        player->ForceNeutralAnim(true);       // アニメも固定
        player->SetShotStop(false);           //　ショットを制御
    }

    //Enemy2のウェーブを初期化
    e2_line_enabled = false;
    e2_group_next = 10.2f;  // Wave2 開始直後から少し遅らせる
    e2_group_id = 0;
    e2_single_next = 11.2f;  // 合間の単発はさらに少し遅らせる
    e2_group_interval = 3.5f;
    e2_single_interval = 1.6f;

    // Enemy5のウェーブを初期化
    e5_started = e5_done = false;
    e5_i_L = e5_i_R = 0;
    e5_next_L = e5_next_R = 0.0f;

    //Enemy6のウェーブを初期化
    e6_started = e6_done = false;
    e6_lane = 0;
    e6_count_in_lane = 0;
    e6_next_at = 0.0f;

    //開始ディレイ
    postwave_start_at = -1.0f;

    //ボス
    boss_spawn_at = -1.0f;
    boss_delay_sec = 9.0f;

    // 既存フラグの初期化：周回用
    wave1_started = wave1_done = false;
    miniboss_spawned = miniboss_done = false;
    postwave_started = postwave_done = false;
    miniboss_start_t = 0.0f;
    wave1_batch = wave1_count = 0;
    wave1_next_at = 0.0f;
}

/// <summary>
/// 終了時処理
/// </summary>
void Stage1::Finalize()
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

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second"></param>
void Stage1::Update(float delta_second)
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

    // クリア判定処理
    UpdateGameStatus(delta_second);

    // スクロールの更新処理
    UpdateBackgroundScroll(delta_second);
    ScrollEffectUpdate(delta_second);   

    // プレイヤーの登場シーン
    AppearancePlayer(delta_second);

    // 登場時のステージラベルの更新処理
    UpdateRabel(delta_second);
}

/// <summary>
/// 描画処理
/// </summary>
void Stage1::Draw()
{
    // 背景スクロール・グリッドの描画
    DrawScrollBackground();
    DrawFrontGrid();

    // オブジェクトの描画処理
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Draw();

    // エフェクトの描画処理
    EffectManager* em = Singleton<EffectManager>::GetInstance();
    em->Draw();

    //ステージラベル：UI
    StageLabel();
}

/// <summary>
/// 次のステージを取得
/// </summary>
/// <param name="player">プレイヤー情報</param>
/// <returns>次のステージ</returns>
StageBase* Stage1::GetNextStage(Player* player)
{
	return new Stage2(player);
}

/// <summary>
/// 背景：背面
/// </summary>
void Stage1::DrawScrollBackground() const
{
    //カメラふんわりオフセット
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
    Vector2D offset_grid = camera_offset * 0.3f; // 奥グリッド
    Vector2D offset_stars = camera_offset * 0.6f; // 星粒子
    Vector2D offset_noise = camera_offset * 0.3f; // ノイズ

    //背景の奥グリッド（ネオングリーン） ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
    {
        int draw_x = x - static_cast<int>(offset_grid.x);
        DrawLine(draw_x, 0, draw_x, D_WIN_MAX_Y, GetColor(0, 200, 120));
    }
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(scroll_back) % grid_size_back;
        sy -= static_cast<int>(offset_grid.y);
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(0, 200, 120));
    }

    //星粒子エフェクト
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    for (const auto& p : star_particles)
    {
        int a = static_cast<int>(p.alpha);
        if (a <= 0) continue;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

        int px = static_cast<int>(p.pos.x - offset_stars.x);
        int py = static_cast<int>(p.pos.y - offset_stars.y);

        DrawLine(px, py, px, py + static_cast<int>(p.length), GetColor(80, 255, 120));
    }

    //ノイズフラッシュ
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    if (rand() % 90 == 0)
    {
        int nx = rand() % D_WIN_MAX_X;
        int ny = rand() % D_WIN_MAX_Y;
        nx -= static_cast<int>(offset_noise.x);
        ny -= static_cast<int>(offset_noise.y);
        DrawBox(nx, ny, nx + 3, ny + 3, GetColor(80, 255, 140), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

/// <summary>
/// 前面グリッド
/// </summary>
void Stage1::DrawFrontGrid() const
{
    // カメラ視差（Stage3と同じ）
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

    Vector2D offset_front = camera_offset * 1.5f; // 手前は動きを大きく

    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);

    // 縦線
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
    {
        int draw_x = x - static_cast<int>(offset_front.x);
        DrawBox(draw_x - 1, 0, draw_x + 1, D_WIN_MAX_Y, GetColor(80, 255, 140), TRUE);
    }
    // 横線（スクロール）
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(scroll_front) % grid_size_front;
        sy -= static_cast<int>(offset_front.y);
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(80, 255, 140), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

/// <summary>
/// 背景スクロールの更新
/// </summary>
/// <param name="delta_second"></param>
void Stage1::ScrollEffectUpdate(float delta_second)
{
   //スクロール
    scroll_back -= delta_second * 110.0f;   // 奥：とてもゆっくり
    scroll_front -= delta_second * 280.0f;  // 手前：奥より少し速い

    // 粒子の更新
    for (auto& p : star_particles)
    {
        p.pos.y += p.velocity.y * delta_second * 0.6f; // 低速化
        p.alpha -= delta_second * 22.0f;               // フェード緩やか
    }
    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p)
            { return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0); }),
        star_particles.end());

    // 粒子補充
    if (star_particles.size() < 80)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 40.0f + GetRand(20));   // 低速
        p.alpha = 120.0f + GetRand(80);
        p.length = 8.0f + GetRand(8);
        p.life = 2.0f + GetRand(100) / 60.0f;
        star_particles.push_back(p);
    }
}

/// <summary>
/// 敵出現
/// </summary>
/// <param name="dt"></param>
void Stage1::EnemyAppearance(float dt)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    // デバッグ：ボスのみ
    if (debug_boss_only) {
        if (!boss_spawned) {
            const float CX = D_WIN_MAX_X * 0.5f;
            boss = objm->CreateObject<Boss1>(Vector2D(CX, -200));
            if (boss) {
                boss->SetPattern(BossPattern::Entrance);
                boss->SetPlayer(player);
            }
            boss_spawned = true;
        }
        return;
    }

    // スタート～6秒まではスポーンしない
    if (stage_timer < intro_delay_sec) return;

    const float CX = D_WIN_MAX_X * 0.5f;
    const float appear_y = -80.0f;
    const float VIEW_X_MIN = CX - 350.0f;  
    const float VIEW_X_MAX = CX + 350.0f;  

    // ─────────────────────────────────────────
    // Wave1：Enemy1 を 3バッチ × 6体
    // 0.30秒おきに1体 → 2秒休み → 次バッチ
    // ─────────────────────────────────────────
    if (!wave1_done) {
        if (!wave1_started) {
            wave1_started = true;
            wave1_batch = 0;
            wave1_count = 0;
            wave1_next_at = stage_timer;

            // 列順プリセット
            int p0[6] = { 2,4,1,5,0,3 };
            memcpy(wave1_pattern0, p0, sizeof(p0));
        }

        if (stage_timer >= wave1_next_at && wave1_batch < 3)
        {
            const int   ENEMY_PER_BATCH = 6;
            const float spacing_x = 90.0f;
            const float target_y = 168.0f + wave1_batch * 30.0f;
            const float appear_time = 1.2f;

            // 列順をパターンで選ぶ
            int col = wave1_pattern0[wave1_count];

            float base_x = CX - (spacing_x * (ENEMY_PER_BATCH - 1) * 0.5f);
            float jitter_x = (GetRand(24) - 12);                // ±12px 
            Vector2D start_pos(base_x + col * spacing_x + jitter_x, appear_y);
            Vector2D target_pos(base_x + col * spacing_x, target_y); 

            auto e1 = objm->CreateObject<Enemy1>(start_pos);
            if (e1) {
                e1->SetAppearParams(start_pos, target_pos, appear_time);
                e1->SetPlayer(player);
            }

            wave1_count++;
            if (wave1_count < ENEMY_PER_BATCH)
            {
                // ランダム間隔
                wave1_next_at = stage_timer + (0.33f + (GetRand(9) * 0.015f));
            }
            else 
            {
                wave1_batch++;
                wave1_count = 0;

                //パターンを回転(先頭を末尾へ)
                std::rotate(std::begin(wave1_pattern0), std::begin(wave1_pattern0) + 1, std::end(wave1_pattern0));

                wave1_next_at = stage_timer + 4.0f; // 休みは既存のまま
            }
        }

        // 終了条件（または wave1_duration_sec の時間切れ）
        if (wave1_batch >= 3 || stage_timer >= intro_delay_sec + wave1_duration_sec) {
            wave1_done = true;
        }
        if (!wave1_done) return;
    }

    // ─────────────────────────────────────────
    // 中ボス（Enemy4 Easy）
    // 倒される or 16秒経過で強制退場して次へ
    // ─────────────────────────────────────────
    if (!miniboss_done) {
        if (!miniboss_spawned) {
            auto e4 = objm->CreateObject<Enemy4>(Vector2D(CX, -240));
            if (e4) {
                e4->SetPlayer(player);
                e4->SetEasyMode(true);  
            }
            enemy4 = e4;
            miniboss_spawned = true;
            miniboss_start_t = stage_timer;
            return; // 生成フレームは終了
        }

        bool timeout = (stage_timer - miniboss_start_t) >= miniboss_timeout_sec;
        if (enemy4) {
            if (enemy4->IsCleared()) {
                miniboss_done = true;
                enemy4 = nullptr;
            }
            else if (timeout) {
                enemy4->ForceRetreat();
                miniboss_done = true;
            }
        }
        else if (timeout) {
            miniboss_done = true; 
        }

        if (!miniboss_done) return; // 中ボス中は他出現なし
    }

    // ─────────────────────────────────────────
    // Wave: Enemy5 斜めフォーメーション（左右）
    // ─────────────────────────────────────────

    if (!e5_done) {
        const int   COUNT_PER_SIDE = 8;   
        const int   COLS = 3;   
        const float APPEAR_T = 1.20f;   //出現の見せ時間
        const float SPACING = 56.0f;    //縦の間隔
        const float SPEED = 100.0f;     //対角移動）
        const float COL_STRIDE = 38.0f; //横の列間隔
        const float INTERVAL = 0.30f;   //発進間隔


        if (!e5_started) { e5_started = true; e5_i_L = e5_i_R = 0; e5_next_L = e5_next_R = stage_timer; }

        // 左上→右下（from_left=true）
        if (e5_i_L < COUNT_PER_SIDE * COLS && stage_timer >= e5_next_L) {
            // line_idx は「先頭から」＝大きい方から
            int emitted = e5_i_L / COLS;          // 何段目を出しているか
            int col = e5_i_L % COLS;              // 列
            int line_idx = (COUNT_PER_SIDE - 1) - emitted;

            Vector2D s(VIEW_X_MIN - 40.0f, -40.0f);
            auto e5 = objm->CreateObject<Enemy5>(s);
            if (e5) {
                e5->SetAppearParams(s, APPEAR_T);
                e5->SetFormationParams(
                    line_idx, COUNT_PER_SIDE, /*from_left=*/true,
                    SPACING, SPEED,
                    /*col_idx=*/col, /*col_total=*/COLS, /*col_stride=*/COL_STRIDE
                );
                e5->SetPlayer(player);
            }
            e5_i_L++;
            e5_next_L = stage_timer + INTERVAL;
        }

        // 右上→左下（from_left=false）
        if (e5_i_R < COUNT_PER_SIDE * COLS && stage_timer >= e5_next_R) {
            int emitted = e5_i_R / COLS;
            int col = e5_i_R % COLS;
            int line_idx = (COUNT_PER_SIDE - 1) - emitted;

            Vector2D s(VIEW_X_MAX + 40.0f, -40.0f);
            auto e5 = objm->CreateObject<Enemy5>(s);
            if (e5) {
                e5->SetAppearParams(s, APPEAR_T);
                e5->SetFormationParams(
                    line_idx, COUNT_PER_SIDE, /*from_left=*/false,
                    SPACING, SPEED,
                    /*col_idx=*/col, /*col_total=*/COLS, /*col_stride=*/COL_STRIDE
                );
                e5->SetPlayer(player);
            }
            e5_i_R++;
            e5_next_R = stage_timer + INTERVAL;
        }

        if (e5_i_L >= COUNT_PER_SIDE * COLS && e5_i_R >= COUNT_PER_SIDE * COLS) {
            e5_done = true;
        }
        if (!e5_done) return;
    }

    // ─────────────────────────────────────────
    // Wave: Enemy6 （3レーン×縦3体）
    // ─────────────────────────────────────────
    // 3レーン位置
    float lanes[3] = {
        VIEW_X_MIN + 120.0f,
        CX,
        VIEW_X_MAX - 120.0f
    };

    if (!e6_done) {
        if (!e6_started) { e6_started = true; e6_lane = 0; e6_count_in_lane = 0; e6_next_at = stage_timer + 1.2f; }

        if (stage_timer >= e6_next_at) {
            Vector2D s(lanes[e6_lane], -80.0f);
            auto e6 = objm->CreateObject<Enemy6>(s);
            if (e6) {
                e6->SetAppearParams(s, /*appear_time=*/1.0f, /*fall_speed=*/85.0f);
                e6->SetPlayer(player);
            }

            e6_count_in_lane++;
            if (e6_count_in_lane < 3) {
                e6_next_at = stage_timer + 0.65f; // 同一レーンで縦に3体（隊列）を出す
            }
            else {
                e6_count_in_lane = 0;
                e6_lane++;
                e6_next_at = stage_timer + 0.9f;  // 次のレーンへ
            }

            if (e6_lane >= 3) e6_done = true;
        }

        if (!e6_done) return;
    }

    // ─────────────────────────────────────────
    // Post-Wave：Enemy2 を左右から（各8体）TopArcExit
    // ─────────────────────────────────────────

    if (!postwave_done) {
        if (postwave_start_at >= 0.0f && stage_timer < postwave_start_at) {
            return; //少し待機
        }

        if (!postwave_started) {
            postwave_started = true;
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 0.6f;   
            spawn_delay_timer_right = 0.6f;
        }
    const int COUNT_PER_SIDE = 12; 

    // 左列
    if (spawn_index_left < COUNT_PER_SIDE) {
        spawn_delay_timer_left -= dt;
        if (spawn_delay_timer_left <= 0.0f) {
            float x = CX - 220.0f - spawn_index_left * 30.0f;
            Vector2D s(x, -80.0f);                         //画面上から
            float base_y_left = 160.0f;
            float rand_y_left = GetRand(60) - 30; //ランダム
            Vector2D t(x, base_y_left + (spawn_index_left % 3) * 40.0f + rand_y_left);

            auto e2 = objm->CreateObject<Enemy2>(s);
            if (e2) {
                e2->SetMode(Enemy2Mode::TopArcExit);       
                e2->SetAppearParams(s, t, 1.50f, true);  // 左→右へカーブ＆退場
                e2->SetPlayer(player);
            }
            spawn_index_left++;
            spawn_delay_timer_left = 0.36f;           
        }
    }

    // 右列
    if (spawn_index_right < COUNT_PER_SIDE) {
        spawn_delay_timer_right -= dt;
        if (spawn_delay_timer_right <= 0.0f) {
            float x = CX + 220.0f + spawn_index_right * 30.0f;
            Vector2D s(x, -80.0f);
            // 右列
            float base_y_right = 180.0f;
            float rand_y_right = GetRand(60) - 30; // -30 ～ +30
            Vector2D t(x, base_y_right + (spawn_index_right % 3) * 40.0f + rand_y_right);

            auto e2 = objm->CreateObject<Enemy2>(s);
            if (e2) {
                e2->SetMode(Enemy2Mode::TopArcExit);
                e2->SetAppearParams(s, t, 1.50f, false);   // 右→左へカーブ＆退場
                e2->SetPlayer(player);
            }
            spawn_index_right++;
            spawn_delay_timer_right = 0.36f;
        }
    }

    if (spawn_index_left >= COUNT_PER_SIDE && spawn_index_right >= COUNT_PER_SIDE) {
        postwave_done = true;
        if (boss_spawn_at < 0.0f) boss_spawn_at = stage_timer + boss_delay_sec; // ★追加：予約
    }
    if (!postwave_done) return;
    }


    // ─────────────────────────────────────────
    // ボス出現（従来どおり）
    // ──────────────────────────────────────────────────────────────────────────────────
    if (boss == nullptr && boss_spawn_at >= 0.0f && stage_timer >= boss_spawn_at) 
    {
        objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss1>(Vector2D(CX, -200));

        if (boss) {
            boss->SetPattern(BossPattern::Entrance);
            boss->SetPlayer(player);
        }
    }

}

/// <summary>
/// クリア：ゲームオーバー判定
/// </summary>
/// <param name="delta_second"></param>
void Stage1::UpdateGameStatus(float delta_second)
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
    if (boss != nullptr && boss->GetIsAlive() == false && is_over == false)
    {
        boss->SetDestroy();
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

    // ステージ終了時の動き（クリア後の待機）
    if (is_clear == true)
    {
        scene_timer += delta_second;

        if (scene_timer >= 2.0f)
        {
            is_finished = true;  // GetNextStage()でStage2に遷移
        }
    }
}

/// <summary>
/// ステージラベル
/// </summary>
void Stage1::StageLabel() const
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

        // グリッチ風ランダムスキャンライン
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
                "Neural Grid", GetColor(255, 255, 255), font_warning);

            DrawStringToHandle(sub_text_x, 370,
                "Eliminate all hostile units.", GetColor(120, 255, 255), font_orbitron);
        }
    }
}

/// <summary>
/// プレイヤー登場演出
/// </summary>
/// <param name="delta_second"></param>
void Stage1::AppearancePlayer(float delta_second)
{
    if (is_player_entering)
    {
        player->SetShotStop(true);
        const float duration = 2.5f;
        player_entry_timer += delta_second;

        float t = player_entry_timer / duration;
        if (t >= 1.0f)
        {
            t = 1.0f;
            is_player_entering = false;
            player->SetMobility(true);
            player->ForceNeutralAnim(false);
            player->SetShotStop(false);
            player->SetNowType(PlayerType::AlphaCode); // 最終的にAlphaに固定
        }
        else
        {
            if (t < 0.2f)
                player->ChangeType(PlayerType::AlphaCode, false);
            else if (t < 0.5f)
                player->ChangeType(PlayerType::OmegaCode, false);
            else
                player->ChangeType(PlayerType::AlphaCode, false);
        }

        // 下降イージング
        float eased = 1.0f - (1.0f - t) * (1.0f - t);
        Vector2D new_pos = entry_start_pos + (entry_end_pos - entry_start_pos) * eased;
        player->SetLocation(new_pos);
    }
}

//ラベル演出中か
bool Stage1::IsStageLabelActive() const 
{
    return (warning_label_state != WarningLabelState::None &&
        warning_label_band_height > 1.0f);  
}