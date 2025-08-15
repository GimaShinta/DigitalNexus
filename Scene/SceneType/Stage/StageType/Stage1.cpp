#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Boss/Boss1.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage1::Stage1(Player* player)
	: StageBase(player)
{
}

Stage1::~Stage1()
{
}

// 初期化処理
void Stage1::Initialize()
{
    // ステージIDの設定
	stage_id = StageID::Stage1;

    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);

    if (player)
    {
        entry_start_pos = Vector2D(D_WIN_MAX_X / 2, 360);             // 中央あたりから
        entry_end_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y - 120); // 通常の定位置

        player_entry_timer = 0.0f; // タイマーリセット

        player->SetMobility(false); // 操作禁止
        player->SetLocation(entry_start_pos); // 最初の位置を設定
        player->ForceNeutralAnim(true); // ← アニメも固定
        player->SetShotStop(false);
    }

    // === 追加: Enemy2 スケジューラ初期化 ===
    e2_line_enabled = false;
    e2_group_next = 10.2f;  // Wave2 開始直後から少し遅らせる
    e2_group_id = 0;

    e2_single_next = 11.2f;  // 合間の単発はさらに少し遅らせる
    e2_group_interval = 3.5f;
    e2_single_interval = 1.6f;

}

// 終了時処理
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

// 更新処理
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

    // プレイヤーの登場シーン
    AppearancePlayer(delta_second);

    // 登場時のステージラベルの更新処理
    UpdateRabel(delta_second);
}

// 描画処理
void Stage1::Draw()
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

    //// ゲームオーバー時の演出
    //if (is_over)
    //{
    //    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    //    SetFontSize(32);
    //    DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
    //    SetFontSize(16);
    //}
}

// 次のステージを取得
StageBase* Stage1::GetNextStage(Player* player)
{
	return new Stage2(player);
}

void Stage1::DrawScrollBackground() const
{
    // === カメラふんわりオフセット ===
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

    // レイヤー別オフセット
    Vector2D layer1_offset = camera_offset * 0.3f;  // 奥グリッド
    Vector2D layer2_offset = camera_offset * 1.5f;  // 手前グリッド
    Vector2D layer_wall_offset = camera_offset * 0.8f; // 壁グリッド

    // === 背景グラデーション（ネオングリーン基調） ===
    for (int y = 0; y < D_WIN_MAX_Y; ++y)
    {
        float t = static_cast<float>(y) / D_WIN_MAX_Y;
        int r = static_cast<int>(5 + (10 - 5) * t);
        int g = static_cast<int>(20 + (60 - 20) * t);
        int b = static_cast<int>(5 + (10 - 5) * t);
        DrawLine(0, y, D_WIN_MAX_X, y, GetColor(r, g, b));
    }

    // === 粒子の生成・描画（ネオングリーン） ===
    while (star_particles.size() < 120)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D((GetRand(200) - 100) * 0.05f, 20.0f + GetRand(30));
        p.alpha = 100.0f + GetRand(155);
        p.length = 6.0f + GetRand(12);
        p.life = 2.0f + (GetRand(100) / 40.0f);
        star_particles.push_back(p);
    }

    for (auto& p : star_particles)
    {
        p.pos += p.velocity * 0.016f;
        p.alpha -= 0.6f;

        if (p.alpha > 0)
        {
            int a = static_cast<int>(p.alpha);
            if (a > 255) a = 255;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
            DrawLine(
                static_cast<int>(p.pos.x - camera_offset.x * 0.6f),
                static_cast<int>(p.pos.y - camera_offset.y * 0.6f),
                static_cast<int>(p.pos.x - camera_offset.x * 0.6f),
                static_cast<int>(p.pos.y + p.length - camera_offset.y * 0.6f),
                GetColor(100, 255, 100));
        }
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p) {
                return (p.pos.y > D_WIN_MAX_Y + 50 || p.alpha <= 0);
            }),
        star_particles.end()
    );

    // === 奥のグリッド(点滅あり) ===
    const int grid_size_back = 40;
    int pulse_alpha = static_cast<int>(80 + 50 * sinf(stage_timer * 3.0f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x - (int)layer1_offset.x, 0, x - (int)layer1_offset.x, D_WIN_MAX_Y, GetColor(0, 255, 120));
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy - (int)layer1_offset.y, D_WIN_MAX_X, sy - (int)layer1_offset.y, GetColor(0, 255, 120));
    }

    // === 壁状の縦グリッド（中央±350px、極太ライン） ===
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    const int wall_grid_size = 50;
    const int wall_thickness = 6; // 極太ライン
    const int left_wall_x = (D_WIN_MAX_X / 2) - 350;
    const int right_wall_x = (D_WIN_MAX_X / 2) + 350;

    for (int wall_x : { left_wall_x, right_wall_x })
    {
        for (int y = -wall_grid_size; y < D_WIN_MAX_Y + wall_grid_size; y += wall_grid_size)
        {
            int sy = y - static_cast<int>(bg_scroll_offset_layer1 * 0.4f) % wall_grid_size;
            DrawBox(wall_x - wall_thickness - (int)layer_wall_offset.x,
                sy - (int)layer_wall_offset.y,
                wall_x + wall_thickness - (int)layer_wall_offset.x,
                sy + wall_grid_size - (int)layer_wall_offset.y,
                GetColor(0, 255, 120), TRUE);
        }
    }

    // === 手前のグリッド ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1 - (int)layer2_offset.x, 0, x + 1 - (int)layer2_offset.x, D_WIN_MAX_Y, GetColor(0, 255, 120), TRUE);
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1 - (int)layer2_offset.y, D_WIN_MAX_X, sy + 1 - (int)layer2_offset.y, GetColor(0, 255, 120), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}







//// 背景スクロールの描画
//void Stage1::DrawScrollBackground() const
//{
//    // === カメラふんわりオフセット（プレイヤー位置に応じて） ===
//    static Vector2D camera_offset(0, 0);               // 描画用のふんわりオフセット
//    static Vector2D camera_target_offset_prev(0, 0);   // 前回のターゲット位置（死亡後用）
//
//    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
//
//    Vector2D camera_target_offset = camera_target_offset_prev; // デフォルトは前回値を使う
//
//    // プレイヤーが存在していれば追従（位置を更新）
//    if (player != nullptr)
//    {
//        Vector2D player_pos = player->GetLocation();
//        camera_target_offset = (player_pos - screen_center) * 0.05f;
//        camera_target_offset_prev = camera_target_offset;
//    }
//    else
//    {
//        // 死亡後も offset_prev を変えない → 固定視点
//        camera_target_offset = camera_target_offset_prev;
//
//    }
//
//    // オフセットをなめらかに反映
//    camera_offset += (camera_target_offset - camera_offset) * 0.1f;
//
//    // レイヤー別オフセット
//    Vector2D layer1_offset = camera_offset * 0.3f; // 奥グリッド
//    Vector2D layer2_offset = camera_offset * 1.5f; // 手前グリッド
//    Vector2D layer3_offset = camera_offset * 0.6f; // 中間
//
//
//    // ====== 以下は描画処理そのまま ======
//    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);
//
//    // === 粒子生成 ===
//    while (star_particles.size() < 100)
//    {
//        StarParticle p;
//        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
//        p.velocity = Vector2D(0, 40.0f + GetRand(60));
//        p.alpha = 150.0f + GetRand(100);
//        p.length = 10.0f + GetRand(10);
//        p.life = 2.0f + (GetRand(100) / 50.0f);
//        star_particles.push_back(p);
//    }
//
//    // === 粒子描画 ===
//    for (auto& p : star_particles)
//    {
//        p.pos.y += p.velocity.y * 0.016f;
//        p.alpha -= 0.5f;
//
//        if (p.length > 0.0f)
//        {
//            int a = static_cast<int>(p.alpha);
//            if (a < 0) a = 0;
//            if (a > 255) a = 255;
//
//            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
//            DrawLine(
//                static_cast<int>(p.pos.x - layer3_offset.x),
//                static_cast<int>(p.pos.y - layer3_offset.y),
//                static_cast<int>(p.pos.x - layer3_offset.x),
//                static_cast<int>(p.pos.y + p.length - layer3_offset.y),
//                GetColor(200, 255, 255));
//        }
//    }
//
//    star_particles.erase(
//        std::remove_if(star_particles.begin(), star_particles.end(), [](const StarParticle& p)
//            {
//                return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0);
//            }),
//        star_particles.end()
//    );
//
//    const int grid_size_back = 40;
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
//    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
//        DrawLine(x - (int)layer1_offset.x, 0, x - (int)layer1_offset.x, D_WIN_MAX_Y, GetColor(0, 100, 255));
//    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
//    {
//        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
//        DrawLine(0, sy - (int)layer1_offset.y, D_WIN_MAX_X, sy - (int)layer1_offset.y, GetColor(0, 100, 255));
//    }
//
//    const int grid_size_front = 80;
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
//    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
//        DrawBox(x - 1 - (int)layer2_offset.x, 0, x + 1 - (int)layer2_offset.x, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
//    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
//    {
//        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
//        DrawBox(0, sy - 1 - (int)layer2_offset.y, D_WIN_MAX_X, sy + 1 - (int)layer2_offset.y, GetColor(180, 0, 255), TRUE);
//    }
//
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
//}


//敵出現処理
void Stage1::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    enemy_spawn_timer += delta_second;

    if (debug_boss_only)
    {
        if (!boss_spawned)
        {
            const float CENTER_X = D_WIN_MAX_X / 2;
            boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
            boss->SetPattern(BossPattern::Entrance);
            boss->SetPlayer(player);
            boss_spawned = true;
        }
        return;
    }
    if (boss != nullptr) return;

    const float CENTER_X = D_WIN_MAX_X / 2;
    const float CENTER_Y = D_WIN_MAX_Y / 2;

    // ==== Wave1 : Enemy1 波状突入 ====
    if (stage_timer < 10.0f)
    {
        const int enemy_count = 6;
        const float spacing_x = 100.0f;
        const float appear_y = -80.0f;
        const float target_y = 150.0f;

        if (!spawning)
        {
            spawning = true;
            spawn_index = 0;
            spawn_delay_timer = 0.0f;
        }

        if (spawning)
        {
            spawn_delay_timer -= delta_second;
            if (spawn_delay_timer <= 0.0f && spawn_index < enemy_count)
            {
                float offset_x = (spawn_index - (enemy_count - 1) / 2.0f) * spacing_x;
                Vector2D start_pos(CENTER_X + offset_x, appear_y - (spawn_index % 2) * 50);
                Vector2D target_pos(CENTER_X + offset_x, target_y + (spawn_index % 2) * 40);

                auto zako = objm->CreateObject<Enemy1>(start_pos);
                if (zako)
                {
                    float appear_time = 0.8f + GetRand(10) * 0.05f;
                    zako->SetAppearParams(start_pos, target_pos, appear_time);
                    zako->SetPlayer(player);
                }
                spawn_index++;
                spawn_delay_timer = 0.4f;
            }
            if (spawn_index >= enemy_count) spawning = false;
        }
    }

    // ==== Wave2 : Enemy2 下から派手登場 ====
    else if (stage_timer < 25.0f)
    {
        // Wave2 に入ったら一度だけ有効化
        if (!e2_line_enabled) {
            e2_line_enabled = true;
            // e2_group_next / e2_single_next は Initialize で設定済み
        }

        // 1) LineRise グループ: 一度に5体生成するが、各個体は順番待ちで 0.45s 間隔で出現
        if (e2_line_enabled && stage_timer >= e2_group_next) {
            const float CENTER_X = D_WIN_MAX_X / 2;
            const int   group_size = 5;
            // グループごとに横幅や到達高さを少し変えて派手さを出す
            float spacing_x = 120.0f + (e2_group_id % 3) * 10.0f;
            float base_x = CENTER_X - (spacing_x * (group_size - 1) * 0.5f);
            float appear_y = D_WIN_MAX_Y + 100.0f;
            float base_target_y = D_WIN_MAX_Y / 2.0f - 70.0f + (GetRand(60) - 30);

            for (int i = 0; i < group_size; ++i) {
                float x = base_x + i * spacing_x;
                Vector2D s(x, appear_y + i * 35.0f); // 後続ほど下から出る＝見た目に“連なってくる”
                Vector2D t(x + (GetRand(20) - 10), base_target_y - i * 12.0f);

                auto e2 = objm->CreateObject<Enemy2>(s);
                if (e2) {
                    // ★Enemy2 側に追加したモード/ライン演出APIを使用
                    e2->SetMode(Enemy2Mode::LineRise);       // 列モード
                    e2->SetLineParams(i, group_size, 0.45f); // i番目が0.45秒刻みで出現
                    e2->SetAppearParams(s, t, 1.2f + (GetRand(20) * 0.01f), true);
                    e2->SetPlayer(player);
                }
            }

            e2_group_id++;
            e2_group_next = stage_timer + e2_group_interval; // 次のグループ時刻
        }

        // 2) 合間の単発（Zako3Like）: 位置と到達時間をランダムにしてリズムを作る
        if (e2_line_enabled && stage_timer >= e2_single_next) {
            const float CENTER_X = D_WIN_MAX_X / 2;
            float x = CENTER_X + (GetRand(560) - 280);   // 広めに散らす
            Vector2D s(x, D_WIN_MAX_Y + 100.0f);
            Vector2D t(x, D_WIN_MAX_Y / 2.0f - 80.0f + (GetRand(60) - 30));

            auto e2 = objm->CreateObject<Enemy2>(s);
            if (e2) {
                e2->SetMode(Enemy2Mode::Zako3Like);                  // 下からZako3風
                e2->SetAppearParams(s, t, 1.1f + (GetRand(40) * 0.01f), true);
                e2->SetPlayer(player);
            }
            e2_single_next = stage_timer + e2_single_interval;       // 次の単発時刻
        }

        if (!zako_spawned)
        {
            zako_spawned = true;
            const int enemy_count = 5;
            const float spacing_x = 150.0f;
            const float appear_y = D_WIN_MAX_Y + 100.0f;
            const float target_y = CENTER_Y - 60.0f;

            for (int i = 0; i < enemy_count; ++i)
            {
                float offset_x = (i - (enemy_count - 1) / 2.0f) * spacing_x;
                Vector2D start_pos(CENTER_X + offset_x, appear_y + i * 40);
                Vector2D target_pos(CENTER_X + offset_x, target_y - i * 20);

                auto e2 = objm->CreateObject<Enemy2>(start_pos);
                if (e2)
                {
                    e2->SetAppearParams(start_pos, target_pos, 1.4f, true);
                    e2->SetPlayer(player);
                }
            }
        }
    }

    // ==== Wave3 : Enemy3 Zako2 + Zako7 複合 ====
    else if (stage_timer < 40.0f)
    {
        if (!spawning_left && enemy_spawn_timer >= 1.5f)
        {
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 0.0f;
            spawn_delay_timer_right = 0.8f;
            spawning_left = spawning_right = true;
            enemy_spawn_timer = 0.0f;
        }

        if (spawning_left)
        {
            spawn_delay_timer_left -= delta_second;
            if (spawn_delay_timer_left <= 0.0f)
            {
                float x = -80.0f;
                float y = CENTER_Y - 80 + spawn_index_left * 80;

                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
                e3->SetMode(ZakoMode::Zako2);
                e3->SetAppearParams(Vector2D(x, y), Vector2D(D_WIN_MAX_X + 80.0f, y + 50), 2.0f, true);
                e3->SetPlayer(player);

                spawn_index_left++;
                if (spawn_index_left >= 3) spawning_left = false;
                else spawn_delay_timer_left = 1.0f;
            }
        }

        if (spawning_right)
        {
            spawn_delay_timer_right -= delta_second;
            if (spawn_delay_timer_right <= 0.0f)
            {
                float x = CENTER_X - 100 + spawn_index_right * 100;
                float y = D_WIN_MAX_Y + 100.0f;

                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
                e3->SetMode(ZakoMode::Zako7);
                e3->SetAppearParams(Vector2D(x, y), Vector2D(x, CENTER_Y - 100), 2.2f, true);
                e3->SetPlayer(player);

                spawn_index_right++;
                if (spawn_index_right >= 3) spawning_right = false;
                else spawn_delay_timer_right = 1.2f;
            }
        }
    }

    // ==== アイテム休憩 ====
    else if (stage_timer < 45.0f)
    {
        if (!spawned_enemy1)
        {
            spawned_enemy1 = true;
            objm->CreateObject<PowerUp>(Vector2D(CENTER_X - 80, CENTER_Y))->SetPlayer(player);
            objm->CreateObject<Shield>(Vector2D(CENTER_X + 80, CENTER_Y))->SetPlayer(player);
        }
    }

    // ==== Boss ====
    else if (stage_timer > 45.0f && !boss_spawned)
    {
        boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
        boss->SetPattern(BossPattern::Entrance);
        boss->SetPlayer(player);
        boss_spawned = true;
    }
}


//void Stage1::EnemyAppearance(float delta_second)
//{
//    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
//
//    // すでにボスが生成済みなら何もしない
//    if (boss_spawned) return;
//
//    // プレイヤーの位置を中央寄りにボスを先に出現させる
//    boss = objm->CreateObject<Boss1>(Vector2D(D_WIN_MAX_X / 2, -200));
//    boss->SetPattern(BossPattern::Entrance);
//    boss->SetPlayer(player);
//
//    boss_spawned = true;  // 一度だけ出現させる
//}





// クリア判定
// クリア判定
void Stage1::UpdateGameStatus(float delta_second)
{
    // 遷移時のノイズ演出（Stage2に合わせた効果）
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
                "Neural Grid", GetColor(255, 255, 255), font_warning);

            DrawStringToHandle(sub_text_x, 370,
                "Eliminate all hostile units.", GetColor(120, 255, 255), font_orbitron);
        }
    }
}

// プレイヤーの登場処理
void Stage1::AppearancePlayer(float delta_second)
{
    // プレイヤーの登場の動き
    if (is_player_entering)
    {
        player->SetShotStop(true);
        const float duration = 2.5f; // ゆっくり2.5秒かけて降下
        player_entry_timer += delta_second;

        float t = player_entry_timer / duration;
        if (t >= 1.0f)
        {
            t = 1.0f;
            is_player_entering = false;
            player->SetMobility(true);
            player->ForceNeutralAnim(false); // ← アニメーション許可
            player->SetShotStop(false);
        }

        // 緩やかに下降するイージング（easeOutQuad）
        float eased = 1.0f - (1.0f - t) * (1.0f - t);
        Vector2D new_pos = entry_start_pos + (entry_end_pos - entry_start_pos) * eased;
        player->SetLocation(new_pos);
    }
}