#include "Stage4.h"
#include "Stage3.h"
#include "../../../../Object/Character/Boss/Boss4.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

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
}

void Stage4::Finalize()
{
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

    // 敵の出現
    EnemyAppearance(delta_second);

    // クリア判定
    UpdateGameStatus(delta_second);

    // スクロールの更新処理
    UpdateBackgroundScroll(delta_second);
}

void Stage4::Draw()
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
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME CLEAR", GetColor(255, 255, 255), TRUE);
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
StageBase* Stage4::GetNextStage(Player* player)
{
    return nullptr;
}

// 背景スクロールの描画
void Stage4::DrawScrollBackground() const
{
    static float time = 0.0f;
    time += 0.05f;

    // 背景色：やや赤みがかったグレー
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(30, 10, 10), TRUE);

    // === 背面レイヤー（奥・細かく・暗め） ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(100, 0, 0));

    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back) {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(100, 0, 0));
    }

    // === 前面レイヤー（太く・明るく・警告感） ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(255, 40, 40), TRUE);

    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front) {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(255, 40, 40), TRUE);
    }

    // === ノイズ演出（崩壊感） ===
    for (int i = 0; i < 5; ++i) {
        if (rand() % 70 == 0) {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;
            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(255, 100, 50), TRUE); // オレンジ警告ドット
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
