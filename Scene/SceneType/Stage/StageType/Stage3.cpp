#include "Stage3.h"
#include "Stage4.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Boss/Boss3.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage3::Stage3(Player* player)
    : StageBase(player)
{
}

Stage3::~Stage3()
{
}

// 初期化処理
void Stage3::Initialize()
{
    stage_id = StageID::Stage3;
}

// 終了時処理
void Stage3::Finalize()
{
}

// 更新処理
void Stage3::Update(float delta_second)
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
    ScrollEffectUpdate(delta_second);
}

// 描画処理
void Stage3::Draw()
{
    // 背景スクロールの描画
    DrawScrollBackground(); 

    // インスタンスの取得
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    EffectManager* em = Singleton<EffectManager>::GetInstance();

    if (draw_animation_first)  // ← このフラグで順序を切り替える
    {
        DrawScrollBackground(); // 背景 & 背面グリッド（奥）

        // グリッドの裏に描画するタイミング：未生成 or 墜落中
        if (boss != nullptr && (!boss->GetGenerate()))
        {
            objm->DrawBoss();
        }

        DrawFrontGrid();  // 前面グリッド
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

        DrawFrontGrid();  // 前面グリッド

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




    //// -------- ステージ演出：Neural Grid --------
    //if (stage_timer < 5.0f)
    //{
    //    int y_top = (360 - 100);  // 固定値でもOK
    //    int y_bottom = (360 + 100);

    //    // パルスライティング（明滅ライン）
    //    float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f; // 0.0?1.0
    //    int pulse_alpha = static_cast<int>(pulse * 180);

    //    SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
    //    DrawLine(0, y_top, 1280, y_top, GetColor(120, 220, 255));
    //    DrawLine(0, y_bottom, 1280, y_bottom, GetColor(120, 220, 255));
    //    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    //    // グリッチ風のランダムスキャンライン（ちらつき演出）
    //    for (int i = 0; i < 8; ++i)
    //    {
    //        int glitch_y = y_top + rand() % (2 * 100);
    //        int glitch_len = 50 + rand() % 100;
    //        int glitch_x = rand() % (1280 - glitch_len);

    //        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
    //        DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
    //        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    //    }

    //    int center_x = D_WIN_MAX_X / 2;

    //    float slide_in = 1.0f;  // 一定表示でアニメしないなら固定値でも
    //    float t = slide_in;

    //    int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
    //    int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

    //    SetFontSize(44);
    //    DrawString(stage_name_x, 320,
    //        "Neural Grid", GetColor(255, 255, 255));
    //    SetFontSize(22);
    //    DrawString(sub_text_x, 370,
    //        "Eliminate all hostile units.", GetColor(120, 255, 255));
    //    SetFontSize(16);
    //}

    DrawGlitchEffect(); // ← 最前面にグリッチ表示

    // クリア時の演出
    if (is_clear)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "NEXT STAGE", GetColor(255, 255, 255), TRUE);
        SetFontSize(16);
    }
    // ゲームオーバー時の演出
    else if (is_over)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
        SetFontSize(16);
    }
}

// 次のステージを取得
StageBase* Stage3::GetNextStage(Player* player)
{
    return new Stage4(player);
}

// 背景スクロールの描画
void Stage3::DrawScrollBackground() const
{
    // === 奥のグリッド描画 ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(100, 0, 100));

    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back) {
        int sy = y - static_cast<int>(scroll_back) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(100, 0, 100));
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

    // === 粒子描画 ===
    for (const auto& p : star_particles)
    {
        int a = static_cast<int>(p.alpha);
        if (a <= 0) continue;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

        DrawLine(static_cast<int>(p.pos.x), static_cast<int>(p.pos.y),
            static_cast<int>(p.pos.x), static_cast<int>(p.pos.y + p.length),
            GetColor(180, 130, 255));
    }

    // === ノイズエフェクト ===
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    for (int i = 0; i < 5; ++i) {
        if (rand() % 60 == 0) {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;
            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(255, 100, 50), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// 背景スクロールの更新処理
void Stage3::ScrollEffectUpdate(float delta_second)
{
    scroll_back -= delta_second * 220.2f;   // 奥グリッドは遅く
    scroll_front -= delta_second * 620.0f;  // 前面グリッドは速く

    // 粒子の更新
    for (auto& p : star_particles)
    {
        p.pos.y += p.velocity.y * delta_second;
        p.alpha -= delta_second * 30.0f;
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p)
            { return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0); }),
        star_particles.end());

    // 粒子追加（負荷軽減のため毎フレーム追加しない）
    if (star_particles.size() < 60)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 60.0f + GetRand(30));
        p.alpha = 100.0f + GetRand(100);
        p.length = 10.0f + GetRand(10);
        p.life = 2.0f + GetRand(100) / 50.0f;
        star_particles.push_back(p);
    }
}

void Stage3::DrawFrontGrid() const
{
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(200, 40, 200), TRUE);

    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front) {
        int sy = y - static_cast<int>(scroll_front) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(200, 40, 200), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// 敵の出現をステージタイマーで管理
void Stage3::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    stage_timer += delta_second;
    enemy_spawn_timer += delta_second;
    enemy3_spawn_timer += delta_second;

    // 1. ボスが出ていればZakoは出現しない
    if (boss_spawned) return;

    if (stage_timer < 4.5f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const int num = 6;
        const float spacing = 100.0f;
        const float appear_y = -100.0f;

        float spawn_interval = my_max(2.0f - stage_timer / 5.0f, 0.5f);
        if (!spawning_zako_left && enemy_spawn_timer >= spawn_interval)
        {
            spawn_index_left = 0;
            spawn_delay_timer_left = 0.0f;
            spawning_zako_left = true;
            enemy_spawn_timer = 0.0f;
        }

        if (!spawning_zako_left) return;

        spawn_delay_timer_left -= delta_second;
        if (spawn_delay_timer_left > 0.0f) return;

        float base_x = 100.0f;
        float dx = spacing;
        float end_base_x = D_WIN_MAX_X / 2 - 180.0f;
        float end_dx = spacing;
        float target_y = 200.0f;

        Vector2D appear_pos(base_x + dx * spawn_index_left, appear_y);
        Vector2D end_pos(end_base_x + end_dx * spawn_index_left, target_y);
        float delay = spawn_index_left * 0.5f;

        auto zako = objm->CreateObject<Enemy3>(appear_pos);
        zako->SetMode(ZakoMode::Zako3);
        zako->SetAppearParams(appear_pos, end_pos, 1.3f + delay, true); // 左から
        zako->SetPlayer(player);

        spawn_index_left++;
        if (spawn_index_left >= num)
        {
            spawning_zako_left = false;
        }
        else
        {
            spawn_delay_timer_left = 0.2f;
        }
    }
    else if (stage_timer < 6.8f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const int num = 6;
        const float spacing = 100.0f;
        const float appear_y = -100.0f;

        float spawn_interval = my_max(2.0f - stage_timer / 5.0f, 0.5f);
        if (!spawning_zako_right && enemy_spawn_timer >= spawn_interval)
        {
            spawn_index_right = 0;
            spawn_delay_timer_right = 0.0f;
            spawning_zako_right = true;
            enemy_spawn_timer = 0.0f;
        }

        if (!spawning_zako_right) return;

        spawn_delay_timer_right -= delta_second;
        if (spawn_delay_timer_right > 0.0f) return;

        float base_x = D_WIN_MAX_X - 100.0f;
        float dx = -spacing;
        float end_base_x = D_WIN_MAX_X / 2 + 120.0f;
        float end_dx = -spacing;
        float target_y = 300.0f;

        Vector2D appear_pos(base_x + dx * spawn_index_right, appear_y);
        Vector2D end_pos(end_base_x + end_dx * spawn_index_right, target_y);
        float delay = spawn_index_right * 0.5f;

        auto zako = objm->CreateObject<Enemy3>(appear_pos);
        zako->SetMode(ZakoMode::Zako3);
        zako->SetAppearParams(appear_pos, end_pos, 1.3f + delay, false); // 右から
        zako->SetPlayer(player);

        spawn_index_right++;
        if (spawn_index_right >= num)
        {
            spawning_zako_right = false;
        }
        else
        {
            spawn_delay_timer_right = 0.2f;
        }
    }
    else if (stage_timer < 15.0f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const float spawn_interval = 4.0f;
        if (enemy3_spawn_timer < spawn_interval) return;

        const int num = 6;
        const float spacing = 80.0f;
        const float delay_per = 0.3f;

        float start_x = D_WIN_MAX_X / 2 - spacing;
        float start_y = -100.0f;  // 左上から出現したいので画面上の外
        float target_start_x = D_WIN_MAX_X / 2 - spacing;
        float target_start_y = 0.0f; // 整列位置のYの開始値

        for (int i = 0; i < num; ++i)
        {
            float x = start_x + spacing * i;
            float y = start_y + spacing * i;  // ← Yも段階的に増加して階段に
            float tx = target_start_x + spacing * i;
            float ty = target_start_y + spacing * i;

            float delay = 1.0f + delay_per * i;

            Vector2D appear_pos(x, y);
            Vector2D target_pos(tx, ty);

            auto zako = objm->CreateObject<Enemy3>(appear_pos);
            zako->SetMode(ZakoMode::Zako2);
            zako->SetAppearParams(appear_pos, target_pos, delay, true);
            zako->SetPlayer(player);
        }

        enemy3_spawn_timer = 0.0f;
    }
    else if (stage_timer < 20.0f)
    {
    }
    else if (stage_timer < 25.0f)
    {
        if (!is_enemy3_group_spawned)
        {
            const float base_y = -50.0f;
            const float center_x = D_WIN_MAX_X / 2;
            const float center_y = D_WIN_MAX_Y / 2 - 30.0f;  // 少し上から始める

            const float step_x = 70.0f;   // 横方向の広がり
            const float step_y = 90.0f;   // 縦方向の段差（大きめ）

            const float delay_step = 0.3f;

            // 左側（X: 中央から左へ、Y: 上→下）
            for (int i = 0; i < 3; ++i)
            {
                float x = center_x - step_x * (i + 1);         // 中央から左に離れていく
                float y = center_y + step_y * i;               // 上から下へ段差
                float delay = 1.0f + delay_step * i;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, base_y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, base_y), Vector2D(x, y), delay, true);
                zako->SetPlayer(player);
            }

            // 右側（X: 中央から右へ、Y: 上→下）
            for (int i = 0; i < 3; ++i)
            {
                float x = center_x + step_x * (i + 1);         // 中央から右に離れていく
                float y = center_y + step_y * i;               // 上から下へ段差
                float delay = 1.0f + delay_step * i;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, base_y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, base_y), Vector2D(x, y), delay, false);
                zako->SetPlayer(player);
            }
            is_enemy3_group_spawned = true;
        }

    }
    else if (stage_timer < 34.5f)
    {
        //is_warning = true;
    }
    else
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        if (boss_spawned) return;

        objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2 + 60, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss3>(Vector2D(670, -200));
        boss->SetPlayer(player);
        //enemy_list.push_back(boss2);
        boss_spawned = true;
        //is_warning = false;
    }
}

// クリア判定
void Stage3::UpdateGameStatus(float delta_second)
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
    if (player != nullptr && player->GetIsAlive() == false && is_clear == false)
    {
        // オブジェクト管理クラスのインスタンスを取得
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
        objm->Finalize();
        is_over = true;
    }

    //// ゲームオーバーだった時の演出準備
    //if (is_over == true)
    //{
    //    scene_timer += delta_second;
    //    //gameover_timer += delta_second;

    //    //if (gameover_timer >= 0.005f)
    //    //{
    //    //    transparent++;
    //    //    gameover_timer = 0.0f;
    //    //}

    //    if (scene_timer >= 5.0f)
    //    {
    //        is_finished = true;
    //    }
    //}

    // ステージ終了時の動き
    if (is_clear == true || is_over == true)
    {
        // 少し待機したら終了
        scene_timer += delta_second;
        trans_timer += delta_second;

        if (trans_timer >= 0.02f)
            if (transparent < 255)
                transparent++;

        if (scene_timer >= 5.0f)
        {
            is_finished = true;
        }
        else
        {
            if (player)
                player->SetShotStop();
        }
    }
}
