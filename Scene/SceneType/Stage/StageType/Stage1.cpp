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
#include "../../../../Utility/ScoreData.h"
#include "../../../../Utility/InputManager.h"

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

    ScoreData* score = Singleton<ScoreData>::GetInstance();
    score->Reset();
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
            score->SetTime(1, game_time_hun, game_time_byou, game_time_miri);
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

void Stage1::ResultDraw(float delta_second)
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

    // =====================
    // ボーナス条件
    // =====================
    int p_life = player->GetLife();

    bool no_damage = (p_life >= 8);
    bool no_special = !player->GetSpecialMove(); // 必殺技未使用
    bool boss_clear = is_clear;

    // 各ボーナス値
    int boss_defeat_bonus = boss_clear ? 1000000 : 0;
    int no_damage_bonus = no_damage ? 1000000 : 0;
    int no_special_bonus = no_special ? 2000000 : 0;

    // =====================
    // 合計スコア（※一度だけ）
    // =====================
    total_score =
        base_score +
        life_bonus +
        time_bonus +
        boss_defeat_bonus +
        no_damage_bonus +
        no_special_bonus;

    // ステージスコア保存
    score->SetStageScore(1, total_score);

    // =====================
    // 表示ライン定義
    // =====================
    struct ResultLine {
        int delay_frame;
        int y_offset;
        std::string label;
        std::string format;
    };

    std::vector<ResultLine> lines;

    // ===== 固定で上に表示 =====
    lines = {
        {  30, -100, "RESULT", "" },
        {  70,  -40, "TOTAL SCORE", "TOTAL SCORE : %.0f" },
        { 110,    0, "LIFE BONUS",  "LIFE BONUS : %d" },
        { 150,   40, "TIME BONUS",  "TIME BONUS : %d" },
    };

    // ここから下を積み上げ
    int delay = 190;
    int y = 80;

    // ===== ボス撃破 =====
    if (boss_clear)
    {
        lines.push_back({
            delay, y,
            "BOSS DEFEAT BONUS",
            "BOSS DEFEAT BONUS : %d"
            });
        delay += 30;
        y += 40;
    }

    // ===== ノーダメージ =====
    if (no_damage)
    {
        lines.push_back({
            delay, y,
            "NO DAMAGE BONUS",
            "NO DAMAGE BONUS : %d"
            });
        delay += 30;
        y += 40;
    }

    // ===== 必殺技未使用 =====
    if (no_special)
    {
        lines.push_back({
            delay, y,
            "NO SPECIAL BONUS",
            "NO SPECIAL BONUS : %d"
            });
        delay += 30;
        y += 40;
    }

    // ===== FINAL（必ず最後）=====
    lines.push_back({
        delay, y,
        "FINAL SCORE",
        "FINAL SCORE : %.0f"
        });

    // ===== Xボタンでリザルト項目スキップ =====
    {
        InputManager* input = Singleton<InputManager>::GetInstance();
        if (input->GetButtonDown(XINPUT_BUTTON_X))
        {
            const float fade_duration = 1.0f; // ResultDraw内の fade_duration と同じ値にする

            // フェードアウト中なら、最後まで飛ばす
            if (result_fadeout_started && !result_ended)
            {
                float last_delay_sec = static_cast<float>(lines.back().delay_frame) / 60.0f;
                result_fadeout_timer = fade_duration + last_delay_sec; // 完了条件まで一気に進める
            }
            // まだ全項目が出ていない：次の行が「完全表示」される所へ飛ぶ
            else if (!result_displayed)
            {
                float now = result_timer;

                // 「次に完全表示させたい行」の目標時間を探す
                float target = now;
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    float delay_sec = static_cast<float>(lines[i].delay_frame) / 60.0f;
                    float fully_time = delay_sec + fade_duration; // この時刻でその行がalpha=255になる

                    if (now < fully_time)
                    {
                        target = fully_time;
                        break;
                    }
                }

                // その時刻まで一気に進める
                result_timer = target;

                // もし最後まで飛んだなら、確実に「表示完了扱い」にしておく
                float last_full = static_cast<float>(lines.back().delay_frame) / 60.0f + fade_duration;
                if (result_timer >= last_full) {
                    result_displayed = true;
                }
            }
            // 全部表示済み：待ち/グリッチも飛ばしてフェードアウトへ
            else
            {
                glitch_started = true;
                glitch_done = true;

                if (!result_fadeout_started) {
                    result_fadeout_started = true;
                    result_fadeout_timer = 0.0f;
                }
            }
        }
    }



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
            else if (line.label == "BOSS DEFEAT BONUS") {
                sprintf_s(value_buf, "%d", boss_defeat_bonus);
            }
            else if (line.label == "NO DAMAGE BONUS") {
                sprintf_s(value_buf, "%d", no_damage_bonus);
            }
            else if (line.label == "NO SPECIAL BONUS") {
                sprintf_s(value_buf, "%d", no_special_bonus);
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
                int line_y = y;
                DrawLine(cx - 600, line_y, cx + 600, line_y, GetColor(255, 255, 255));

                if (alpha == 255 && !result_fadeout_started) {
                    result_displayed = true;
                }
            }
        }
    }

    // ===== スキップ案内UI（Bボタン）=====
    {
        // どの状態かで文言を変える
        const char* hint = nullptr;

        // まだ項目が出切ってない → SKIP
        if (!result_displayed && !result_fadeout_started)
            hint = "X : SKIP";
        // 出切ったが、まだメニューに行ってない → NEXT（待ち/グリッチ飛ばし用）
        else if (result_displayed && !result_fadeout_started)
            hint = "X : NEXT";
        // フェードアウト中は出さない（好みで "A : FAST" でもOK）
        else
            hint = nullptr;

        if (hint)
        {
            // 位置：中央黒帯の右下
            const int cx = D_WIN_MAX_X / 2;
            int x = (cx + 350) - 220;          // 黒帯右端 - 余白
            int y = D_WIN_MAX_Y - 60;

            // 点滅（控えめ）
            int pulse = (GetNowCount() % 60 < 30) ? 255 : 140;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse);
            DrawStringToHandle(x, y, hint, GetColor(120, 255, 255), font_orbitron);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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

void Stage1::UpdateResultMenu(float dt)
{
    // 演出タイマ
    result_menu_timer += dt;
    result_menu_blink_t += dt;

    // パネルの開き（0.40sで1.0へ）
    const float OPEN_SEC = 0.40f;
    if (result_menu_open_t < 1.0f) {
        result_menu_open_t += dt / OPEN_SEC;
        if (result_menu_open_t > 1.0f) result_menu_open_t = 1.0f;
    }

    // パネルが7割開いたら文字フェード開始（0.30s）
    const float TEXT_START = 0.70f;
    const float TEXT_FADE_SEC = 0.30f;
    if (result_menu_open_t >= TEXT_START && result_menu_text_t < 1.0f) {
        result_menu_text_t += dt / TEXT_FADE_SEC;
        if (result_menu_text_t > 1.0f) result_menu_text_t = 1.0f;
    }

    // 入力（Aを推奨表示。Space対応は残す）
    InputManager* input = Singleton<InputManager>::GetInstance();
    if (input->GetButtonDown(XINPUT_BUTTON_A) || input->GetKeyDown(KEY_INPUT_SPACE)) {
        is_finished = true; // GameMainScene側でセレクトへ遷移
    }
}


// 横伸びのアウトバック（少し勢いを付ける）
static float EaseOutBack(float t, float s = 1.70158f) {
    float u = t - 1.0f;
    return 1.0f + (u * u * ((s + 1.0f) * u + s));
}

void Stage1::DrawResultMenu()
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


//ラベル演出中か
bool Stage1::IsStageLabelActive() const 
{
    return (warning_label_state != WarningLabelState::None &&
        warning_label_band_height > 1.0f);  
}

