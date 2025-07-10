#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
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
}

// 終了時処理
void Stage1::Finalize()
{
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
    if (stage_timer < 5.0f)
    {
        int y_top = (360 - 100);  // 固定値でもOK
        int y_bottom = (360 + 100);

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
            int glitch_y = y_top + rand() % (2 * 100);
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (1280 - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        int center_x = D_WIN_MAX_X / 2;

        float slide_in = 1.0f;  // 一定表示でアニメしないなら固定値でも
        float t = slide_in;

        int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
        int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

        SetFontSize(44);
        DrawString(stage_name_x, 320,
            "Neural Grid", GetColor(255, 255, 255));
        SetFontSize(22);
        DrawString(sub_text_x, 370,
            "Eliminate all hostile units.", GetColor(120, 255, 255));
        SetFontSize(16);
    }

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
StageBase* Stage1::GetNextStage(Player* player)
{
	return new Stage2(player);
}

// 背景スクロールの描画
void Stage1::DrawScrollBackground() const
{
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);

    // === 粒子生成 ===
    while (star_particles.size() < 100)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 40.0f + GetRand(60)); // やや速め
        p.alpha = 150.0f + GetRand(100); // より光る
        p.length = 10.0f + GetRand(10);
        p.life = 2.0f + (GetRand(100) / 50.0f);
        star_particles.push_back(p);
    }

    // === 粒子描画 ===
    for (auto& p : star_particles)
    {
        p.pos.y += p.velocity.y * 0.016f;
        p.alpha -= 0.5f;

        if (p.length > 0.0f)
        {
            int a = static_cast<int>(p.alpha);
            if (a < 0) a = 0;
            if (a > 255) a = 255;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

            // より明るい色
            DrawLine(static_cast<int>(p.pos.x),
                static_cast<int>(p.pos.y),
                static_cast<int>(p.pos.x),
                static_cast<int>(p.pos.y + p.length),
                GetColor(200, 255, 255));
        }
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(), [](const StarParticle& p)
            {
                return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0);
            }),
        star_particles.end()
    );

    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(0, 100, 255));
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(0, 100, 255));
    }

    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(180, 0, 255), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// 敵の出現をstage_timerで管理
void Stage1::EnemyAppearance(float delta_second)
{
    enemy_spawn_timer += delta_second;

    float spawn_interval = 2.0f - (stage_timer / 5.0f);
    if (spawn_interval < 0.5f) spawn_interval = 0.5f;

    if (enemy_spawn_timer >= spawn_interval)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        // 【0?24秒】：縦から直線的に出現
        if (stage_timer < 24.0f)
        {
            const int lane_x[] = { 250, 350, 450, 550, 650, 750, 850 };
            float x = static_cast<float>(lane_x[GetRand(7)]);
            Enemy1* enemy = objm->CreateObject<Enemy1>(Vector2D(x, -40));
            enemy->SetPattern(Enemy1Pattern::MoveStraight);
            enemy->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        // 【25?48秒】：左右から回り込み移動
        else if (stage_timer < 48.0f)
        {
            const int lane_y[] = { 40, 120, 200 };
            float y = static_cast<float>(lane_y[GetRand(3)]);
            bool from_left = fmod(stage_timer, 20.0f) < 10.0f;
            Vector2D pos = from_left ? Vector2D(270.0f, y) : Vector2D(1000.0f, y);
            Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

            Enemy1* zako = objm->CreateObject<Enemy1>(pos);
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        // 【49?59秒】：突撃系パターン（止まって撃つ or 突進）
        else if (stage_timer < 60.0f)
        {
            const int lane_x[] = { 350, 450, 550, 650, 750, 850 };
            float x = static_cast<float>(lane_x[GetRand(6)]);
            Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, -40));
            Enemy1Pattern pattern = (GetRand(2) == 0)
                ? Enemy1Pattern::MoveAndStopShoot
                : Enemy1Pattern::DiveOnce;
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        else if (stage_timer < 65.0f) // 【60?65秒】：ZIgzag + MoveStraight 混合
        {
            for (int i = 0; i < 5; ++i)
            {
                float x = 300.0f + GetRand(400);
                Enemy1Pattern pattern = (i % 2 == 0) ? Enemy1Pattern::ZIgzag : Enemy1Pattern::MoveStraight;

                Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, -60.0f - i * 30));
                zako->SetPattern(pattern);
                zako->SetPlayer(player);
                //enemy_list.push_back(zako);
            }
        }

        else if (stage_timer < 70.0f) // 【65?70秒】：左右から順番に登場
        {
            const int lane_y[] = { 40, 120, 200 };
            float y = static_cast<float>(lane_y[GetRand(3)]);
            bool from_left = fmod(stage_timer, 20.0f) < 10.0f;
            Vector2D pos = from_left ? Vector2D(270.0f, y) : Vector2D(1000.0f, y);
            Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

            Enemy1* zako = objm->CreateObject<Enemy1>(pos);
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        else if (stage_timer < 74.0f) // 【70?74秒】：中央をかすめるように一気に突進
        {
            for (int i = 0; i < 3; ++i)
            {
                bool from_left = (GetRand(2) == 0);
                float y = 100.0f + i * 80.0f;

                Vector2D pos = from_left
                    ? Vector2D(-60.0f, y)
                    : Vector2D(D_WIN_MAX_X + 60.0f, y);

                Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

                Enemy1* zako = objm->CreateObject<Enemy1>(pos);
                zako->SetPattern(pattern);
                zako->SetPlayer(player);
                //enemy_list.push_back(zako);
            }
        }

        //else if (stage_timer < 77.0f) // 【74?77秒】：グリッチ演出 → 全ザコが退避
        //{
        //    for (auto& enemy : enemy_list)
        //    {
        //        if (!enemy || enemy->is_destroy) continue;

        //        Zako* zako = dynamic_cast<Zako*>(enemy);
        //        if (zako == nullptr) continue;

        //        ZakoPattern current = zako->GetPattern();

        //        if (current == ZakoPattern::MoveAndStopShoot ||
        //            current == ZakoPattern::DiveOnce ||
        //            current == ZakoPattern::RotateAndShoot ||
        //            current == ZakoPattern::PauseThenRush ||
        //            current == ZakoPattern::LeftMove ||
        //            current == ZakoPattern::RightMove ||
        //            current == ZakoPattern::Formation)
        //        {
        //            zako->SetPattern(ZakoPattern::RetreatUp);
        //        }
        //    }
        //}

        // 【77秒以降】：ボス戦突入 + アイテム演出
        else
        {
            if (!boss_spawned)
            {
                objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
                objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2 + 60, 120))->SetPlayer(player);

                boss = objm->CreateObject<Boss1>(Vector2D(670, -200));
                boss->SetPattern(BossPattern::Entrance);
                boss->SetPlayer(player);
                //enemy_list.push_back(boss1);

                boss_spawned = true;
                //is_warning = false;
            }
        }

        enemy_spawn_timer = 0.0f;
    }
}

// クリア判定
void Stage1::UpdateGameStatus(float delta_second)
{
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

        if (trans_timer >= 0.01f)
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
