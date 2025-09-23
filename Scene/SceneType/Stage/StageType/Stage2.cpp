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

    // Mid関連の初期化は削除（撤廃のため）
    // s2_mid_spawned = s2_mid_done = false; などは消す


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

    // ★ Warning 演出は使わないため、ラベル状態もクリア寄りに
    warning_label_state = WarningLabelState::None;  // ← 警告帯撤廃:contentReference[oaicite:4]{index=4}:contentReference[oaicite:5]{index=5}


    // ★ ボス登場演出 初期値
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
}

// 描画
void Stage2::Draw()
{
    // 背景
    DrawScrollBackground();

    // 手前の太いグリッド（これが見えなかった原因：未呼び出し）:contentReference[oaicite:7]{index=7}
    DrawFrontGrid();

    // オブジェクト
    Singleton<GameObjectManager>::GetInstance()->Draw();

    // エフェクト
    Singleton<EffectManager>::GetInstance()->Draw();



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



    // ステージ演出ラベル（WARNING含む）
    StageLabel();
}

// 次のステージ
StageBase* Stage2::GetNextStage(Player* player)
{
    return new Stage3(player);
}

// ===== 背景：視差スクロール付きのニューログリッド =====
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


void Stage2::ScrollEffectUpdate(float delta_second)
{
    // ここを “遅く”
    scroll_back -= delta_second * 110.0f;  // 200 → 110
    scroll_front -= delta_second * 280.0f;  // 600 → 280

    for (auto& p : star_particles) {
        p.pos.y += p.velocity.y * delta_second * 0.6f; // 1.0 → 0.6（落下もゆっくり）
        p.alpha -= delta_second * 22.0f;               // 30 → 22（フェード緩やか）
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
  // Wave1：Enemy8 三機隊列（中央だけ少し前）を“複数回”レーン出現
  // =========================
    if (!wave1_started) {
        wave1_started = true;
        wave1_done = false;
        wave1_group = 0;                   // ★ 初期化
        wave1_next_at = stage_timer + 1.6f;  // ★ 開幕を少し遅らせる
    }

    if (!wave1_done) {
        if (stage_timer >= wave1_next_at) {
            const float W = (float)D_WIN_MAX_X;
            const float H = (float)D_WIN_MAX_Y;

            // 三機フォーメーションの横オフセット
            constexpr float SPACING_X = 72.0f;  // ← 100 → 72 に縮める（お好みで 64?84 程度）
            constexpr float START_Y = -80.0f;
            constexpr float TARGET_Y = 170.0f;
            constexpr float CENTER_Y = TARGET_Y + 18.0f;
            constexpr float APPEAR_T = 1.25f;

            // ── レーン設計 ─────────────────────────────
            // 三機横並びの“半幅”ぶん余白をとって、安全に収まるレーン中心を計算
            const float formation_half = SPACING_X;   // 左右 ±SPACING_X に置く
            const float margin = 15.0f;
            const float left = formation_half + margin;
            const float right = W - (formation_half + margin);

            // ★ 追加：レーンの展開幅を圧縮（値を小さくするほど“狭く”寄る）
            const float LANE_SPAN_SCALE = 0.30f; // 例: 0.6（0<scale<=1）
            const float mid = (left + right) * 0.5f;
            const float half_span = (right - left) * 0.5f * LANE_SPAN_SCALE;
            const float leftN = mid - half_span;
            const float rightN = mid + half_span;

            // レーン数（お好みで 3?7 くらいまで増やせます）
            constexpr int LANE_N = 5;

            // レーン中心座標（★ left/right ではなく leftN/rightN を使う）
            auto laneCenter = [&](int laneIdx)->float {
                if (LANE_N <= 1) return (leftN + rightN) * 0.5f;
                return leftN + (rightN - leftN) * (float)laneIdx / (float)(LANE_N - 1);
                };

            // 見た目に単調にならない並び順（中央→左端→右端→左中→右中）
            auto laneOrder = [&](int k)->int {
                switch (k % LANE_N) {
                case 0: return 2; // center
                case 1: return 0; // far left
                case 2: return 4; // far right
                case 3: return 1; // mid left
                default:return 3; // mid right
                }
                };

            // 今回出すレーン
            const int   laneIdx = laneOrder(wave1_group);
            const float CX = laneCenter(laneIdx);

            // 三機の相対配置（中央だけ少し前へ）
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
                wave1_next_at += wave1_interval;  // ★ 間隔をあけてもう一度
            }
            else {
                wave1_done = true;                // ★ Wave1 完了、次へ進める
            }
        }

        // Wave1 が続いている間はここで止める（後続Waveに進めない）
        if (!wave1_done) return;
    }


    // =========================
    // Wave2：斜め流し（左上→右下）
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

            float sx = 120.0f + (wave2_count % 4) * 60.0f; // 120,180,240,300 ...
            float sy = -60.0f  - (wave2_count / 4) * 40.0f; // -60,-100,-140

            Vector2D start_pos(sx, sy);
            Vector2D target_pos(sx + DX * 6.0f, sy + DY * 6.0f); // 斜め大移動

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
    // ===== Wave2 終了後：中ボス Enemy7×2 追加 =====
    if (wave2_done && !enemy7_done)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
        const float CX = D_WIN_MAX_X * 0.5f;
        const float W = (float)D_WIN_MAX_X;
        const float H = (float)D_WIN_MAX_Y;

        if (!enemy7_spawned) {
            enemy7_spawned = true;
            enemy7_start_t = stage_timer;

            // 左右の徘徊範囲を定義（Xは半画面、Yは浅め）
            RectF leftArea{ 40.0f,   H * 0.20f,  CX - 24.0f, H * 0.38f };
            RectF rightArea{ CX + 24.0f, H * 0.20f,  W - 40.0f, H * 0.38f };

            // 左：やや左上から入場
            enemy7_left = objm->CreateObject<Enemy7>(Vector2D(CX - 220.0f, -220.0f));
            if (enemy7_left) {
                enemy7_left->SetPlayer(player);
                enemy7_left->SetWanderBounds(leftArea);
                enemy7_left->SetLabel("LEFT");
                // 難易度スイッチがあればここで
                // enemy7_left->SetEasyMode(player && player->IsEasyMode());
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
            enemy7_left->ForceRetreat(); //left_done = true;
        }
        if (!right_done && (stage_timer - enemy7_start_t) >= enemy7_timeout_sec) {
            enemy7_right->ForceRetreat(); //right_done = true;
        }

        if (left_done && right_done) {
            // 念のため掃除
           // if (enemy7_left)  enemy7_left->SetDestroy();
           // if (enemy7_right) enemy7_right->SetDestroy();
            enemy7_done = true;
        }
        else {
            // ★ 中ボスが生きている間はここで止める
            return;
        }
    }
    // =========================
    // Wave3：中央から小集団 × 3
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

//void Stage2::EnemyAppearance(float dt)
//{
//    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
//
//    // 0) 序盤は演出のみ（~5.0s）
//    if (stage_timer < 5.0f) return;
//
//    const float CX = D_WIN_MAX_X * 0.5f;
//    const float appear_y_top = -80.0f;
//    // ─────────────────────────────────────
//    // Wave1：逐次スポーン（最初の方式の応用・短縮）
//    // 5.0s開始 → フェーズ0（左右スウィープ）→ 小休止 → フェーズ1（上レイン）
//    // それぞれ1体ずつタイマで出すので一度に終わらない
//    // ─────────────────────────────────────
//    if (!s2_wave1_done)
//    {
//        const float CX = D_WIN_MAX_X * 0.5f;
//
//        if (!s2_wave1_started && stage_timer >= 5.0f) {
//            s2_wave1_started = true;
//            s2_wave1_phase = 0;
//            s2_wave1_i = 0;
//            s2_wave1_next = stage_timer;  // すぐ最初の1体
//        }
//
//        if (s2_wave1_started && stage_timer >= s2_wave1_next)
//        {
//            if (s2_wave1_phase == 0) {
//                // === フェーズ0：左右スウィープ（交互に1体ずつ） ===
//                constexpr int   N = 6;     // 合計体数
//                constexpr float INTERVAL = 0.22f;  // 1体ごとの間隔
//                constexpr float APPEAR_T = 0.95f;  // 到達時間
//
//                bool from_left = (s2_wave1_i % 2 == 0);
//                float lane_t = float(s2_wave1_i % 6) / 5.0f;      // 0..1
//                float sy = 130.0f + 120.0f * lane_t;
//
//                float sx = from_left ? -90.0f : (D_WIN_MAX_X + 90.0f);
//                float tx = CX + (from_left ? -1.0f : +1.0f) * (200.0f - 120.0f * lane_t);
//                float ty = sy + 18.0f;
//
//                Vector2D s(sx, sy), t(tx, ty);
//                if (auto e1 = objm->CreateObject<Enemy1>(s)) {
//                    e1->SetAppearParams(s, t, APPEAR_T);
//                    e1->SetPlayer(player);
//                }
//
//                s2_wave1_i++;
//                if (s2_wave1_i < N) {
//                    s2_wave1_next = stage_timer + INTERVAL;
//                }
//                else {
//                    // 次フェーズへ（小休止して切り替え）
//                    s2_wave1_phase = 1;
//                    s2_wave1_i = 0;
//                    s2_wave1_next = stage_timer + 0.6f;
//                }
//            }
//            else if (s2_wave1_phase == 1) {
//                // === フェーズ1：上レイン（1体ずつ落ちてくる） ===
//                constexpr int   N = 12;
//                constexpr float INTERVAL = 0.18f;
//                constexpr float APPEAR_T = 0.85f;
//
//                float t01 = (s2_wave1_i + 0.5f) / float(N);          // 0..1
//                float sx = 120.0f + (D_WIN_MAX_X - 240.0f) * t01;   // 画面上端の等間隔
//                float sy = -80.0f;
//
//                // 着地点は微蛇行：一直線に並ばない
//                float tx = sx + 36.0f * std::sinf(t01 * 6.28318f);
//                float ty = 180.0f + 28.0f * std::cosf(t01 * 6.28318f);
//
//                Vector2D s(sx, sy), t(tx, ty);
//                if (auto e1 = objm->CreateObject<Enemy1>(s)) {
//                    e1->SetAppearParams(s, t, APPEAR_T);
//                    e1->SetPlayer(player);
//                }
//
//                s2_wave1_i++;
//                if (s2_wave1_i < N) {
//                    s2_wave1_next = stage_timer + INTERVAL;
//                }
//                else {
//                    s2_wave1_done = true;  // Wave1完了
//                }
//            }
//        }
//
//        if (!s2_wave1_done) return;
//    }
//
//
//    // ─────────────────────────────────────
//  // Wave2：Enemy2（20.0s ～ 35.0s）… Snakeのみ
//  // ─────────────────────────────────────
//    if (!s2_wave2_done)
//    {
//        // 開始トリガ（20sから）
//        if (!s2_wave2_started && stage_timer >= 20.0f)
//        {
//            s2_wave2_started = true;
//            s2_e2_group_id = 0;
//            s2_e2_group_next = stage_timer + 0.2f; // 少し間を置いて最初のグループ
//            s2_e2_single_next = -1.0f;
//            s2_e2_single_used = false;
//        }
//
//        // 時間制限（保険）
//        if (stage_timer >= 35.0f) s2_wave2_done = true;
//
//        // まだWave2を進めるなら
//        if (s2_wave2_started && !s2_wave2_done)
//        {
//            // ===== 2-A) グループ（9体 Snake 隊列）=====
//            if (stage_timer >= s2_e2_group_next)
//            {
//                constexpr int   GROUP_SIZE = 9;
//                constexpr float LINE_INTERVAL = 0.14f; // i毎の遅延
//                constexpr float APPEAR_TIME = 1.00f; // 到達時間（固定時間）
//                constexpr float SPACING_X = 50.0f;
//                const     float APPEAR_Y = D_WIN_MAX_Y + 90.0f;
//
//                // アンカー（左/中/右）巡回
//                const float CX = D_WIN_MAX_X * 0.5f;
//                float anchor_x;
//                switch (s2_e2_group_id % 3) {
//                case 0: anchor_x = CX;            break;
//                case 1: anchor_x = CX - 100.0f;   break;
//                default:anchor_x = CX + 100.0f;   break;
//                }
//                float base_x = anchor_x - (SPACING_X * (GROUP_SIZE - 1) * 0.5f);
//
//                // Snake オフセット（上下うねり）
//                auto snakeOfs = [&](int i)->Vector2D {
//                    float c = i - (GROUP_SIZE - 1) * 0.5f;
//                    float ox = 18.0f * sinf(c * 0.9f);
//                    float oy = -10.0f * i + 48.0f * sinf(c * 0.9f + DX_PI / 3);
//                    return Vector2D(ox, oy);
//                    };
//
//                for (int i = 0; i < GROUP_SIZE; ++i)
//                {
//                    Vector2D ofs = snakeOfs(i);
//                    float x = base_x + i * SPACING_X + ofs.x;
//
//                    Vector2D s(x, APPEAR_Y + i * 16.0f);
//                    Vector2D t(x, D_WIN_MAX_Y / 2.0f - 120.0f + ofs.y); // レーンYは上固定
//
//                    auto e2 = objm->CreateObject<Enemy2>(s);
//                    if (e2)
//                    {
//                        e2->SetMode(Enemy2Mode::LineRise);
//                        e2->SetLineParams(i, GROUP_SIZE, LINE_INTERVAL);
//                        e2->SetAppearParams(s, t, APPEAR_TIME, true);
//                        e2->SetPlayer(player);
//                    }
//                }
//
//                // ★次スケジュールを未来に進める（同フレームで再生成を防止）
//                const float group_finish = stage_timer + (GROUP_SIZE - 1) * LINE_INTERVAL + APPEAR_TIME;
//                s2_e2_group_next = group_finish + 0.6f;  // 次グループまで少し休む
//                s2_e2_single_next = group_finish + 0.2f;  // 単発は直後に1回だけ
//                s2_e2_single_used = false;                // 単発リセット
//
//                s2_e2_group_id++;
//                // グループ回数上限で軽量化（必要に応じて調整）
//                if (s2_e2_group_id >= 4) {
//                    s2_wave2_done = true;
//                }
//            }
//
//            // ===== 2-B) 単発：直前グループの芯に 1回だけ =====
//            if (!s2_wave2_done && s2_e2_single_next > 0.0f && !s2_e2_single_used && stage_timer >= s2_e2_single_next)
//            {
//                const float CX = D_WIN_MAX_X * 0.5f;
//
//                // 直前グループのアンカー位置に合わせる（左/中/右）
//                float anchor_x;
//                switch ((s2_e2_group_id > 0 ? s2_e2_group_id - 1 : 0) % 3) {
//                case 0: anchor_x = CX;            break;
//                case 1: anchor_x = CX - 100.0f;   break;
//                default:anchor_x = CX + 100.0f;   break;
//                }
//
//                // Stairs撤廃：単発も上レーン固定
//                float single_y = D_WIN_MAX_Y / 2.0f - 125.0f;
//
//                Vector2D s(anchor_x, D_WIN_MAX_Y + 90.0f);
//                Vector2D t(anchor_x, single_y);
//
//                auto e2 = objm->CreateObject<Enemy2>(s);
//                if (e2)
//                {
//                    e2->SetMode(Enemy2Mode::Zako3Like);
//                    e2->SetAppearParams(s, t, 1.00f, true);
//                    e2->SetPlayer(player);
//                }
//
//                // ★このグループの単発は消化済みに
//                s2_e2_single_used = true;
//                s2_e2_single_next = -1.0f; // 再発しない
//            }
//        }
//
//        if (!s2_wave2_done) return;
//    }
//
//
//    // ─────────────────────────────────────
//    // Wave3：Enemy3 左右交互（35.0s ～ 50.0s）
//    // ─────────────────────────────────────
//    if (!s2_wave3_done)
//    {
//        if (!s2_wave3_started)
//        {
//            s2_wave3_started = true;
//            s2_left_idx = s2_right_idx = 0;
//            s2_left_delay = s2_right_delay = 0.0f;
//        }
//
//        // 左列 6体
//        if (s2_left_idx < 6)
//        {
//            s2_left_delay -= dt;
//            if (s2_left_delay <= 0.0f)
//            {
//                float base_x = 100.0f;
//                float spacing = 100.0f;
//                Vector2D appear_pos(base_x + spacing * s2_left_idx, -100.0f);
//                Vector2D end_pos(D_WIN_MAX_X / 2 - 180.0f + spacing * s2_left_idx, 220.0f);
//                float delay = s2_left_idx * 0.4f;
//
//                auto z = objm->CreateObject<Enemy3>(appear_pos);
//                z->SetMode(ZakoMode::Zako3);
//                z->SetAppearParams(appear_pos, end_pos, 1.2f + delay, true);
//                z->SetPlayer(player);
//
//                s2_left_idx++;
//                s2_left_delay = 0.25f;
//            }
//        }
//
//        // 右列 6体
//        if (s2_right_idx < 6)
//        {
//            s2_right_delay -= dt;
//            if (s2_right_delay <= 0.0f)
//            {
//                float base_x = D_WIN_MAX_X - 100.0f;
//                float spacing = -100.0f;
//                Vector2D appear_pos(base_x + spacing * s2_right_idx, -100.0f);
//                Vector2D end_pos(D_WIN_MAX_X / 2 + 120.0f + spacing * s2_right_idx, 300.0f);
//                float delay = s2_right_idx * 0.4f;
//
//                auto z = objm->CreateObject<Enemy3>(appear_pos);
//                z->SetMode(ZakoMode::Zako3);
//                z->SetAppearParams(appear_pos, end_pos, 1.2f + delay, false);
//                z->SetPlayer(player);
//
//                s2_right_idx++;
//                s2_right_delay = 0.25f;
//            }
//        }
//        // Wave3 のおわり
//        if (s2_left_idx >= 6 && s2_right_idx >= 6)
//        {
//            if (!s2_wave3_done) {
//                s2_wave3_done = true;
//                s2_wave3_done_time = stage_timer; // ★ここで終了時刻を記録
//            }
//        }
//
//        if (!s2_wave3_done) return;
//
//    }
//
//  
//    // ─────────────────────────────────────
//    // ラスト：Boss2 直接スポーン（Warning撤廃）
//    // Wave3 終了から s2_boss_delay_after_wave3 秒 後に出す
//    // ─────────────────────────────────────
//    if (!s2_boss_spawned && s2_wave3_done
//        && s2_wave3_done_time >= 0.0f
//        && stage_timer >= s2_wave3_done_time + s2_boss_delay_after_wave3)
//    {
//        const float CX = D_WIN_MAX_X * 0.5f;
//
//        if (!stage2boss2_spawned)
//        {
//            stage2boss2_spawned = true;
//            boss_appear_state = BossAppearState::Spawning; // Warningは使わない
//
//            boss2 = objm->CreateObject<Boss2>(Vector2D(CX, 240.0f));
//            if (boss2) {
//                boss2->Initialize();
//                boss2->SetPlayer(player);
//            }
//            // 必要なら演出
//            // flash_request = true;
//        }
//
//        s2_boss_spawned = true;
//        return;
//    }
//
//
//}


// 敵の出現（演出フロー込み）
//void Stage2::EnemyAppearance(float delta_second)
//{
//    enemy_spawn_timer += delta_second;
//    boss_appear_timer += delta_second;
//
//    switch (boss_appear_state)
//    {
//        case BossAppearState::Waiting:
//            // ★ 5.0秒で警告へ（元はここで即スポーンしていた）:contentReference[oaicite:4]{index=4}
//            if (stage_timer >= 5.0f)
//            {
//                boss_appear_state = BossAppearState::Warning;
//                boss_appear_timer = 0.0f;
//                bg_speed_scale = 1.2f; // 少し加速
//                // TODO: サイレンSEなど
//            }
//            break;
//
//        case BossAppearState::Warning:
//            // 2.0秒ほど警告表示 → スポーンへ
//            if (boss_appear_timer >= 2.0f)
//            {
//                boss_appear_state = BossAppearState::Spawning;
//                boss_appear_timer = 0.0f;
//                bg_speed_scale = 2.0f; // さらに加速
//
//                if (!stage2boss2_spawned)
//                {
//                    stage2boss2_spawned = true;
//                    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
//                    boss2 = objm->CreateObject<Boss2>(Vector2D(640.0f, 240.0f));
//                    boss2->Initialize();
//                    boss2->SetPlayer(player);
//                }
//
//                // 登場瞬間の白フラッシュ
//                flash_request = true;
//            }
//            break;
//
//        case BossAppearState::Spawning:
//            // Boss2 のイントロ（ズーム＋パーツ集結）が終わったら戦闘へ
//            if (boss2 != nullptr && !boss2->IsIntroActive()) // ★ Boss2側アクセサで判定:contentReference[oaicite:5]{index=5}:contentReference[oaicite:6]{index=6}
//            {
//                boss_appear_state = BossAppearState::Active;
//                boss_appear_timer = 0.0f;
//                bg_speed_scale = 1.0f; // 平常に戻す
//                flash_request = true;  // 戦闘開始フラッシュ
//                // TODO: BGM切り替え
//            }
//            break;
//
//        case BossAppearState::Active:
//            // 通常進行
//            break;
//    }
//}

// クリア・ゲームオーバーの判定
void Stage2::UpdateGameStatus(float delta_second)
{
    // 遷移ノイズ
    if (entry_effect_playing)
    {
        entry_effect_timer += delta_second;
        if (entry_effect_timer >= 1.0f) entry_effect_playing = false;
    }

    // ボスが倒れたらクリア
    if (boss2 != nullptr && boss2->GetIsAlive() == false && is_over == false)
    {
        boss2->SetDestroy();
        is_clear = true;
    }

    //if (boss != nullptr && boss->GetIsAlive() == false && !is_over)
    //{
    //    boss->SetDestroy();
    //    is_clear = true;
    //}

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
// Stage2.cpp : StageLabel を差し替え
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

//void Stage2::UpdateRabel(float delta_second)
//{
//    // ラベルのスライド／表示／退場アニメ（既存）
//    const float slideDur = 0.45f;
//    const float showDur = 1.10f;
//
//    switch (warning_label_state)
//    {
//        case WarningLabelState::SlideIn:
//            warning_label_timer += delta_second;
//            // 0→1 で帯が伸びる
//            {
//                float t = warning_label_timer / slideDur;
//                if (t > 1.0f) t = 1.0f;
//                // 0→240px くらいまで伸ばす
//                warning_label_band_height = 240.0f * t;
//                if (warning_label_timer >= slideDur)
//                {
//                    warning_label_timer = 0.0f;
//                    warning_label_state = WarningLabelState::Displaying;
//                }
//            }
//            break;
//
//        case WarningLabelState::Displaying:
//            warning_label_timer += delta_second;
//            // 高さは一定
//            if (warning_label_timer >= showDur)
//            {
//                warning_label_timer = 0.0f;
//                warning_label_state = WarningLabelState::SlideOut;
//            }
//            break;
//
//        case WarningLabelState::SlideOut:
//            warning_label_timer += delta_second;
//            {
//                float t = warning_label_timer / slideDur;
//                if (t > 1.0f) t = 1.0f;
//                // 240→0 に縮む
//                warning_label_band_height = 240.0f * (1.0f - t);
//                if (warning_label_timer >= slideDur)
//                {
//                    warning_label_timer = 0.0f;
//                    warning_label_state = WarningLabelState::None;
//                    warning_label_band_height = 0.0f;
//                }
//            }
//            break;
//
//        case WarningLabelState::None:
//        default:
//            // 何もしない
//            break;
//    }
//}

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


void Stage2::DrawFrontGrid() const
{
    // Stage3::DrawFrontGrid 相当（色だけ青寄りに変更）:contentReference[oaicite:5]{index=5}
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

