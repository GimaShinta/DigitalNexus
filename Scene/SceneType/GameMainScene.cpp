#include "GameMainScene.h"
#include "../../Object/GameObjectManager.h"
#include "../../Utility/EffectManager.h"
#include "../../Utility/SEManager.h"
#include "../../Utility/ScoreData.h"
#include "../../Object/Character/Player/Player.h"
#include "Stage/StageType/Stage1.h"
#include "Stage/StageType/Stage2.h"
#include "Stage/StageType/Stage3.h"
#include "Stage/StageType/Stage4.h"

GameMainScene::GameMainScene()
{
}

GameMainScene::~GameMainScene()
{
}

// 初期化処理
void GameMainScene::Initialize()
{
    // オブジェクト管理クラスのインスタンスを取得
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    player = objm->CreateObject<Player>(Vector2D(D_WIN_MAX_X / 2, (D_WIN_MAX_Y / 2) + 220.0f));

    current_stage = new Stage1(player);

    current_stage->Initialize();

    // BGM読み込み（初回のみ）
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    stage_bgm1 = rm->GetSounds("Resource/sound/bgm/stage/stage1_2.mp3");
    stage_bgm3 = rm->GetSounds("Resource/sound/bgm/stage/Cybernetic.mp3");

    stage_bgm4 = rm->GetSounds("Resource/sound/bgm/stage/Last_Boss.mp3"); // 任意のファイル
    se_warning = rm->GetSounds("Resource/sound/se/battle/Warning.mp3");       // 警告音SE

    se_charge = rm->GetSounds("Resource/sound/se/effect/audiostock_1133382.mp3");
    ChangeVolumeSoundMem(255 * 100 / 100, se_charge);

    cursor_se = rm->GetSounds("Resource/sound/se/se_effect/cursor.mp3");

    //サイドパネル画像
    obi_handle = rm->GetImages("Resource/Image/BackGround/Main/Obi_1.png")[0];

    //フォント
    font_digital = CreateFontToHandle("Orbitron", 28, 6, DX_FONTTYPE_ANTIALIASING);
    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 20, 6, DX_FONTTYPE_ANTIALIASING);

    m_menuFontHandle = CreateFontToHandle("Orbitron", 36, 6); // メニュー専用フォント

    // ステージ1用BGMを再生
    current_bgm_handle = stage_bgm1;

    ChangeVolumeSoundMem(255 * 50 / 100, current_bgm_handle);

    PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);

    //AnimationManager による事前アニメ再生（0.01秒、画面外で一度だけ再生）
    EffectManager* effe = Singleton<EffectManager>::GetInstance();
    SEManager* se = Singleton<SEManager>::GetInstance();

    effe->LoadAllEffects(); // ★ここで明示的に画像を先読みさせる！
    se->LoadSE();
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType GameMainScene::Update(float delta_second)
{
	//InputManager* input = Singleton<InputManager>::GetInstance();

	//if (input->GetKeyDown(KEY_INPUT_SPACE))
	//	return eSceneType::eRanking;

    // 入力インスタンスの取得
    InputManager* input = Singleton<InputManager>::GetInstance();

    // ポーズ機能
    if (input->GetButtonDown(XINPUT_BUTTON_START) ||
        input->GetKeyDown(KEY_INPUT_P)) {
        isPaused = !isPaused;
        m_selectedIndex = 0;
    }

    // ポーズ状態ではないときの処理（メイン）
    if (!isPaused)
    {
        black_in_timer += delta_second;
        line_effect_timer += delta_second;
        transparent = 0;
        // ポーズ中は何も更新しない（またはポーズ画面の更新のみ）
        if (current_stage)
        {
            // エフェクトの更新処理
            EffectManager* effe = Singleton<EffectManager>::GetInstance();
            effe->Update(delta_second);

            // 進行中のステージの更新処理
            current_stage->Update(delta_second);

            // 進行中のステージがステージ４になったら警告演出をする
            if (current_stage->GetStageID() == StageID::Stage4)
            {
                // 毎フレーム加算（delta_second を使うならそのまま代入）
                red_alpha_timer += delta_second * 1.0f * DX_PI; // 数値を上げれば速く、下げればゆっくり

                WarningUpdate(delta_second);
            }

            // ステージの切り替えOKなら切り替える
            if (current_stage->IsFinished() || input->GetKeyDown(KEY_INPUT_1))
            {
                if (current_stage->IsClear() == true || input->GetKeyDown(KEY_INPUT_1))
                {
                    if (current_stage->GetStageID() == StageID::Stage3)
                    {
                        black_fade_timer += delta_second;
                        if (alpha >= 255)
                        {
                            StopSoundMem(stage_bgm3); // ← ステージ3のBGMを明示的に停止

                            StageBase* next_stage = current_stage->GetNextStage(player);

                            current_stage->Finalize();
                            delete current_stage;
                            current_stage = nullptr;

                            if (next_stage != nullptr)
                            {
                                current_stage = next_stage;
                                current_stage->Initialize();

                                // ステージ4用のBGMを再生（必要なら stage_bgm4 を定義）
                                current_bgm_handle = stage_bgm4;  // または stage_bgm4
                                ChangeVolumeSoundMem(255 * 90 / 100, current_bgm_handle);
                            }
                            else
                            {
                                return eSceneType::eTitle;
                            }
                        }
                    }
                    else if (current_stage->GetStageID() == StageID::Stage4)
                    {
                        end_sequence_started = true; // 終了演出を開始

                        if (player)
                            player->SetShotStop(true);

                        if (end_sequence_started)
                        {
                            end_sequence_timer += delta_second;

                            const float fade_duration = 2.0f; // 2秒フェード

                            // 徐々に暗くする（非依存）
                            if (end_sequence_timer >= 1.0f && end_sequence_timer < 1.0f + fade_duration)
                            {
                                float t = (end_sequence_timer - 1.0f) / fade_duration;
                                if (t > 1.0f) t = 1.0f;
                                end_fade_alpha = static_cast<int>(255 * t);
                            }
                            else if (end_sequence_timer >= 1.0f + fade_duration)
                            {
                                end_fade_alpha = 255;
                            }

                            // 4秒後にタイトルへ遷移
                            if (end_sequence_timer >= 8.0f)
                            {
                                current_stage->Finalize();
                                delete current_stage;
                                current_stage = nullptr;
                                return eSceneType::eTitle;
                            }
                        }
                    }
                    else
                    {
                        StageBase* next_stage = current_stage->GetNextStage(player);

                        current_stage->Finalize();
                        delete current_stage;
                        current_stage = nullptr;

                        if (next_stage != nullptr)
                        {
                            // === ステージの切替とBGM処理 ===
                            current_stage = next_stage;
                            current_stage->Initialize();

                            // ステージ3に到達した場合のみBGM切替
                            if (current_stage->GetStageID() == StageID::Stage3)
                            {
                                StopSoundMem(current_bgm_handle); // 現在のBGMを停止
                                current_bgm_handle = stage_bgm3;  // ステージ3用BGMに切り替え
                                ChangeVolumeSoundMem(255 * 90 / 100, current_bgm_handle);
                                PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);
                            }
                        }
                        else
                        {
                            return eSceneType::eTitle;
                        }
                    }
                }
                else if (current_stage->IsOver() == true)
                {
                    current_stage->Finalize();
                    delete current_stage;
                    current_stage = nullptr;

                    return eSceneType::eTitle;
                }
            }
        }
    }
    else
    {
        // ポーズのとき画面を不透明にする
        pause_timer += delta_second;
        if (pause_timer >= 0.01f)
        {
            transparent++;
            pause_timer = 0.0f;
        }
        if (transparent >= 100)
        {
            transparent = 100;
        }

        InputManager* input = Singleton<InputManager>::GetInstance();

        // 上下キー or コントローラ十字で選択
        bool movedUp = input->GetKeyDown(KEY_INPUT_UP) || input->GetKeyDown(KEY_INPUT_W) ||
            input->GetButtonDown(XINPUT_BUTTON_DPAD_UP);
        bool movedDown = input->GetKeyDown(KEY_INPUT_DOWN) || input->GetKeyDown(KEY_INPUT_S) ||
            input->GetButtonDown(XINPUT_BUTTON_DPAD_DOWN);

        // スティックY軸の入力を反映（上下で判定、クールダウン制御付き）
        static float stickCooldown = 0.0f;
        stickCooldown -= delta_second;

        Vector2D stick = input->GetLeftStick(); // 左スティックY軸：上→正、下→負
        const float STICK_THRESHOLD = 0.5f;     // 感度調整
        const float COOLDOWN_TIME = 0.2f;       // 連続入力防止

        // スティックでのカーソル操作
        if (stickCooldown <= 0.0f)
        {
            if (stick.y > STICK_THRESHOLD)
            {
                movedUp = true;
                stickCooldown = COOLDOWN_TIME;
            }
            else if (stick.y < -STICK_THRESHOLD)
            {
                movedDown = true;
                stickCooldown = COOLDOWN_TIME;
            }
        }

        // カーソルの上限を超えたら下限に移動（その逆も）
        if (movedUp)
        {
            PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
            m_selectedIndex = (m_selectedIndex + 1) % 2;
        }
        if (movedDown)
        {
            PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
            m_selectedIndex = (m_selectedIndex + 1) % 2;
        }

        // 決定（スペース or Aボタン）
        if (!m_startTransitioning && (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A)))
        {
            if (m_selectedIndex == 0) 
            {
                isPaused = false;
            }
            else if (m_selectedIndex == 1) 
            {
                // オブジェクト管理クラスのインスタンスを取得
                GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
                objm->Finalize();

                return eSceneType::eTitle;
            }
        }
    }

    // ======= スコアログのスライド演出更新 =======
    for (auto& log : score_logs)
    {
        if (log.y_offset < 0.0f)
        {
            // イージング風に滑らかに補正（速度を比例減衰）
            log.y_offset += (-log.y_offset) * 10.0f * delta_second;

            // 誤差吸収（0に近づいたら0にする）
            if (log.y_offset > -0.5f)
                log.y_offset = 0.0f;
        }
    }

    // ======= 新しいスコアの追加検出とログ生成 =======
    ScoreData* score = Singleton<ScoreData>::GetInstance();
    const auto& all_scores = score->GetScoreData();

    while (previous_score_count < all_scores.size())
    {
        float new_score = all_scores[static_cast<size_t>(previous_score_count)];

        // スコアログ用メッセージ作成
        char buf[64];
        sprintf_s(buf, sizeof(buf), "Score +%.0f", new_score);

        // ★ 先に最大行数チェック（10行超えたら古いのから削除）
        if (score_logs.size() >= 10)
        {
            score_logs.erase(score_logs.begin()); // 一番古いログを削除
        }

        // 既存ログを下に押し出す
        for (auto& log : score_logs)
        {
            log.y_offset = -20.0f; // スライド開始位置
        }

        // 新しいログを追加（スライドなし）
        ScoreLog new_log;
        new_log.text = buf;
        new_log.y_offset = 0.0f;
        score_logs.push_back(new_log);

        previous_score_count += 1.0f;
    }

    // シールドON検出
    static bool prev_shield = false;
    bool now_shield = player->GetShieldOn();
    if (!prev_shield && now_shield) {
        effect_shield_on = true;
        effect_timer = 0.0f;
    }
    if (prev_shield && !now_shield) {
        effect_shield_off = true;
        effect_timer = 0.0f;
    }
    prev_shield = now_shield;

    // パワーアップ検出（1フレームのみ）
    static int prev_power = 1;
    int now_power = player->GetPowerd();
    if (now_power > prev_power) {
        effect_powerup = true;
        effect_timer = 0.0f;
    }
    prev_power = now_power;

    // タイマー進行
    effect_timer += delta_second;
    if (effect_timer > effect_duration) {
        effect_shield_on = effect_shield_off = effect_powerup = false;
    }

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void GameMainScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "GameMainScene", GetColor(255, 255, 255));
#endif
    if (current_stage)
    {
        EffectManager* anim = Singleton<EffectManager>::GetInstance();
        anim->Draw();
        current_stage->Draw();
        DrawUI();

        if (current_stage->GetStageID() == StageID::Stage3)
        {
            if (current_stage->IsClear())
            {
                if (black_fade_timer >= 0.01f)
                {
                    black_fade_timer = 0.0f;
                    alpha += 2;
                }
                //int alpha = static_cast<int>((black_fade_timer / black_fade_duration) * 255.0f);
                if (alpha > 255) alpha = 255;

                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                if (alpha >= 255)
                {
                    current_stage->SetFinished();
                }
            }
        }
        else if (current_stage->GetStageID() == StageID::Stage4)
        {
            if (black_in_timer2 >= 8.0f)
            {
                if (black_in_timer >= 0.01f)
                {
                    black_in_timer = 0.0f;
                    alpha--;
                }
            }

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE); // ← 背景を最初に描く
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // ← これを追加！（ここ重要！） ?

            if (warning_state != WarningState::None)
            {
                float flash_alpha = 200.0f * sinf(warning_timer * 10.0f);
                if (flash_alpha < 0) flash_alpha = 0;

                // ===== 警告終了間際に帯と文字をフェードアウト =====
                float fade_start_time = warning_duration - 1.0f;
                int band_alpha = 255;
                if (warning_timer >= fade_start_time)
                {
                    float t = (warning_duration - warning_timer);
                    if (t < 0.0f) t = 0.0f;
                    band_alpha = static_cast<int>(255 * t);  // 0?255に減少
                }

                // ===== 背景フラッシュ（赤） =====
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)flash_alpha);
                DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 50, 50), TRUE);

                SetDrawBlendMode(DX_BLENDMODE_ALPHA, band_alpha);

                const int band_height = 120;
                const char* warn_text = "!! WARNING !!";
                const char* scroll_text = "!! SECURITY WARNING  FINAL PROTOCOL ACTIVE  SYSTEM LOCKDOWN ENGAGED !! - !! SECURITY WARNING  FINAL PROTOCOL ACTIVE  SYSTEM LOCKDOWN ENGAGED !!";
                int scroll_speed = 200;

                int text_width = GetDrawStringWidthToHandle(scroll_text, strlen(scroll_text), font_orbitron);
                int scroll_color = GetColor(255, 100 + GetRand(80), 80);

                float scale = 3.0f;
                int base_w = GetDrawStringWidthToHandle(warn_text, strlen(warn_text), font_warning);
                int draw_w = (int)(base_w * scale);
                int draw_h = (int)(32 * scale);

                int x = (D_WIN_MAX_X - draw_w) / 2;
                int y = (D_WIN_MAX_Y - draw_h) / 2;

                int offset_x = GetRand(5) - 2;
                int offset_y = GetRand(3) - 1;

                int pulse = (GetNowCount() % 100 < 50) ? 255 : 100;
                int color = GetColor(255, pulse, pulse);

                // 中央巨大 WARNING
                DrawExtendStringToHandle(x + offset_x, y + offset_y, scale, scale, warn_text, color, font_warning);

                // === 上帯背景とライン ===
                DrawBox(0, 0, D_WIN_MAX_X, band_height, GetColor(30, 0, 0), TRUE);
                DrawLine(0, band_height - 2, D_WIN_MAX_X, band_height - 2, GetColor(255, GetRand(100), 100));

                float scroll_scale = 1.8f;
                int scroll_font_h = (int)(32 * scroll_scale);
                int scroll_x_top = static_cast<int>(D_WIN_MAX_X - (warning_timer * scroll_speed));
                if (scroll_x_top + text_width * scroll_scale > 0)
                {
                    DrawExtendStringToHandle(scroll_x_top, band_height / 2 - scroll_font_h / 2,
                        scroll_scale, scroll_scale, scroll_text, scroll_color, font_orbitron);
                }

                // === 下帯背景とライン ===
                DrawBox(0, D_WIN_MAX_Y - band_height, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(30, 0, 0), TRUE);
                DrawLine(0, D_WIN_MAX_Y - band_height + 1, D_WIN_MAX_X, D_WIN_MAX_Y - band_height + 1, GetColor(255, GetRand(100), 100));

                int scroll_x_bot = static_cast<int>(-(text_width * scroll_scale) + (warning_timer * scroll_speed));
                if (scroll_x_bot < D_WIN_MAX_X)
                {
                    DrawExtendStringToHandle(scroll_x_bot, D_WIN_MAX_Y - band_height + band_height / 2 - scroll_font_h / 2,
                        scroll_scale, scroll_scale, scroll_text, scroll_color, font_orbitron);
                }

                // ===== ノイズエフェクト（帯上） =====
                for (int i = 0; i < 15; ++i)
                {
                    int nx = GetRand(D_WIN_MAX_X);
                    int ny = GetRand(band_height);
                    int nl = GetRand(40) + 20;

                    DrawBox(nx, ny, nx + nl, ny + 2, GetColor(100 + GetRand(100), 0, 0), TRUE);
                    DrawBox(nx, D_WIN_MAX_Y - band_height + ny, nx + nl, D_WIN_MAX_Y - band_height + ny + 2, GetColor(100 + GetRand(100), 0, 0), TRUE);
                }

                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }

            if (alpha < 0)
            {
                alpha = 0;
                black_in_timer2 = 0.0f;
            }
        }

        if (end_sequence_started)
        {
            // フェードアウト（黒）
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, end_fade_alpha);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // メッセージ表示（フェードと合わせて浮かび上がる）
            if (end_sequence_timer >= 1.0f)
            {
                int alpha = (int)((end_sequence_timer - 1.0f) * 255);
                if (alpha > 255) alpha = 255;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

                const char* msg = "GAME CLEAR";
                int text_w = GetDrawStringWidthToHandle(msg, strlen(msg), font_digital);
                int x = (D_WIN_MAX_X - text_w) / 2;
                int y = D_WIN_MAX_Y / 2;

                DrawStringToHandle(x, y, msg, GetColor(255, 255, 255), font_digital);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }

        if (isPaused)
        {
            const int cx = D_WIN_MAX_X / 2;
            const int cy = D_WIN_MAX_Y / 2 - 20;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            //DrawBox(400, 200, 880, D_WIN_MAX_Y - 200, GetColor(255, 255, 255), FALSE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            const char* menuItems[] = {
                "START GAME",
                "BACK TITLE",
            };

            for (int i = 0; i < 2; ++i)
            {
                int y = 300 + i * 50;
                int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
                int x = (D_WIN_MAX_X - textWidth) / 2;

                if (i == m_selectedIndex)
                {
                    // =========================
                    // 背景ハイライトバー（画面端まで）
                    // =========================
                    int barHeight = 40;
                    int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50); // パルス
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
                    DrawBox(
                        0, y - 5,
                        D_WIN_MAX_X, y + barHeight,
                        GetColor(0, 200, 255), TRUE
                    );

                    // =========================
                    // 両端のエッジエフェクト（流れる光）
                    // =========================
                    int edgeWidth = 80;
                    int edgeHeight = 4;
                    int scrollSpeed = 2;
                    int edgeOffset = (GetNowCount() * scrollSpeed) % (D_WIN_MAX_X + edgeWidth * 2);

                    // 左から右へ流れる（2個表示してループ感を出す）
                    for (int j = 0; j < 2; ++j)
                    {
                        int edgeX = edgeOffset - edgeWidth * j;
                        SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
                        DrawBox(
                            edgeX, y + 10,
                            edgeX + edgeWidth, y + 10 + edgeHeight,
                            GetColor(200, 255, 255), TRUE
                        );
                        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                    }

                    // =========================
                    // テキスト（アウトライン＋グロー）
                    // =========================
                    int offsetX = (rand() % 3) - 1;
                    int offsetY = (rand() % 3) - 1;

                    DrawStringToHandle(x + offsetX - 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX + 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX, y + offsetY - 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX, y + offsetY + 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);

                    int glow = (int)((sinf(GetNowCount() / 30.0f) + 1.0f) * 127);
                    DrawStringToHandle(x + offsetX, y + offsetY, menuItems[i], GetColor(100 + glow, 255, 255), m_menuFontHandle);
                }
                else
                {
                    DrawStringToHandle(x, y, menuItems[i], GetColor(180, 180, 180), m_menuFontHandle);
                }
            }
        }
    }
#if _DEBUG
    if(current_stage)
        DrawFormatString(0, 30, GetColor(255, 255, 255), "Stage %d", current_stage->GetStageID());
#endif
}

// 終了時処理（使ったインスタンスの削除とか）
void GameMainScene::Finalize()
{
    GameObjectManager::GetInstance()->Finalize();
    //ScoreData::GetInstance()->
    StopSoundMem(current_bgm_handle); // 現在のBGMを停止

    if (font_orbitron != -1) {
        DeleteFontToHandle(font_orbitron);
        font_orbitron = -1;
    }
    if (font_warning != -1) {
        DeleteFontToHandle(font_warning);
        font_warning = -1;
    }
    if (font_digital != -1) {
        DeleteFontToHandle(font_digital);
        font_digital = -1;
    }
    if (m_menuFontHandle != -1) {
        DeleteFontToHandle(m_menuFontHandle);
        m_menuFontHandle = -1;
    }

    // トータルスコアをランキングにエントリー
    ScoreData::GetInstance()->ScoreEntrie(ScoreData::GetInstance()->GetTotalScore());
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType GameMainScene::GetNowSceneType() const
{
	return eSceneType::eGameMain;
}

void GameMainScene::AddScoreLog(const std::string& text)
{
    // 新しいログを上に追加（スライド無し）
    ScoreLog log;
    log.text = text;
    log.y_offset = 0.0f;
    score_logs.push_back(log);

    // 最大10行保持 → 古いものから削除
    if (score_logs.size() >= 10)
    {
        score_logs.erase(score_logs.begin()); // 先頭（古い）を削除
    }

    // 既存ログはスライド開始（下に押し出される）
    for (auto& log : score_logs)
    {
        log.y_offset = -20.0f;  // 毎フレームで補正される
    }
}

void GameMainScene::DrawUI()
{
    // === 左の黒帯 ===
    DrawGraph(0, 0, obi_handle, TRUE);

    // === 右の黒帯（左右反転）===
    DrawTurnGraph(D_WIN_MAX_X - 290, 0, obi_handle, TRUE);

    // === 左のサイドパネル（サイバー風） ===
    {
        int left_x1 = 0;
        int left_x2 = (D_WIN_MAX_X / 2) - 350;
        int panel_color = GetColor(10, 10, 30);
        int neon_color = GetColor(0, 255, 255);

        // 背景
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(left_x1, 0, left_x2, D_WIN_MAX_Y, panel_color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        // 上下ネオンライン
        DrawLine(left_x1, 0, left_x2, 0, neon_color);
        DrawLine(left_x1, D_WIN_MAX_Y - 1, left_x2, D_WIN_MAX_Y - 1, neon_color);

        if (current_stage)
        {
            float scanline_speed = 0.0f;
            int color = 0;

            if (current_stage->GetStageID() == StageID::Stage4)
            {
                scanline_speed = 600.0f;  // 例：1秒で600px下へ移動
                color = GetColor(255, 0, 0);
            }
            else
            {
                scanline_speed = 60.0f;  // 例：1秒で60px下へ移動
                color = GetColor(0, 150, 255);
            }
            int scan_y = static_cast<int>(line_effect_timer * scanline_speed) % D_WIN_MAX_Y;
            DrawLine(left_x1, scan_y, left_x2, scan_y, color);
        }
    }

    // === 右のサイドパネル（サイバー風） ===
    {
        int right_x1 = (D_WIN_MAX_X / 2) + 350;
        int right_x2 = D_WIN_MAX_X;
        int panel_color = GetColor(10, 10, 30);
        int neon_color = GetColor(0, 255, 255);

        // 背景
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(right_x1, 0, right_x2, D_WIN_MAX_Y, panel_color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        // 上下ネオンライン
        DrawLine(right_x1, 0, right_x2, 0, neon_color);
        DrawLine(right_x1, D_WIN_MAX_Y - 1, right_x2, D_WIN_MAX_Y - 1, neon_color);

        if (current_stage)
        {
            float scanline_speed = 0.0f; 
            int color = 0;

            if (current_stage->GetStageID() == StageID::Stage4)
            {
                scanline_speed = 600.0f;  // 例：1秒で600px下へ移動
                color = GetColor(255, 0, 0);
            }
            else
            {
                scanline_speed = 60.0f;  // 例：1秒で60px下へ移動
                color = GetColor(0, 150, 255);
            }
            int scan_y = static_cast<int>(line_effect_timer * scanline_speed) % D_WIN_MAX_Y;
            DrawLine(right_x1, scan_y, right_x2, scan_y, color);
        }
    }

    if (current_stage && current_stage->GetStageID() == StageID::Stage4) 
    {
        // α値を0～50の範囲で往復させる（sin波を使う）
        int alpha = static_cast<int>((sinf(red_alpha_timer) + 1.0f) * 0.5f * 130.0f);  // → 0～50に

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, 290, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        DrawBox(990, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    }
    // Draw() のサイドパネル描画後に追加
    for (int y = 0; y < 720; y += 4)
    {
        int alpha = (y % 8 == 0) ? 40 : 20;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, y, 290, y + 2, GetColor(100, 255, 255), TRUE); // 左パネル
        DrawBox(990, y, 1280, y + 2, GetColor(100, 255, 255), TRUE); // 右パネル（例）
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ==== トータルスコア（左上） ====
    {
        ScoreData* score = Singleton<ScoreData>::GetInstance();
        float total_score = score->GetTotalScore();

        int x = 30, y = 80;
        int w = 240, h = 80;

        DrawLine(x, y, x + w, y, GetColor(0, 255, 255));         // 上
        DrawLine(x, y + h, x + w, y + h, GetColor(0, 255, 255)); // 下

        // 見出し
        DrawStringToHandle(x + 10, y + 8, "TOTAL SCORE", GetColor(0, 255, 255), font_orbitron);

        // 数値部分の文字列を一度生成
        char score_str[64];
        sprintf_s(score_str, sizeof(score_str), "%.0f", total_score);

        // 幅を測って右寄せ位置を計算（x+w?padding?幅）
        int score_width = GetDrawStringWidthToHandle(score_str, strlen(score_str), font_digital);
        int score_x = x + w - 20 - score_width; // ←右から20px余白

        // 右寄せ描画
        DrawStringToHandle(score_x, y + 40, score_str, GetColor(255, 255, 100), font_digital);
    }

    // ==== スコアログ（左下） ====
    int log_base_x = 30;
    int log_base_y = D_WIN_MAX_Y - 400;
    int line_height = 40;

    int count = static_cast<int>(score_logs.size());
    for (int i = 0; i < count; ++i)
    {
        const auto& log = score_logs[count - 1 - i];
        int draw_y = log_base_y + static_cast<int>(i * line_height + log.y_offset);

        // ライン描画
        DrawLine(log_base_x - 10, draw_y - 2, log_base_x + 210, draw_y - 2, GetColor(0, 255, 255));

        // 「Score 」部分と「+xxxx」部分を分ける
        std::string label = "Score ";
        std::string value;

        // 「Score +xxxx」から+以降を切り出す
        const char* plus_pos = strchr(log.text.c_str(), '+');
        if (plus_pos)
        {
            value = plus_pos; // "+1234" など
        }
        else
        {
            value = ""; // 念のため
        }

        // 「+1234」の横幅を測る
        int value_width = GetDrawStringWidthToHandle(value.c_str(), (int)value.size(), font_orbitron);

        // 右端を log_base_x + 200 に固定して、数値部分を右寄せ
        int value_x = log_base_x + 200 - value_width;

        // 左側に「Score 」を描画
        DrawStringToHandle(log_base_x, draw_y, label.c_str(), GetColor(0, 255, 255), font_orbitron);

        // 右寄せで「+1234」描画
        DrawStringToHandle(value_x, draw_y, value.c_str(), GetColor(0, 255, 255), font_orbitron);
    }

    // ==== LIFE - STOCK（右上） ====
    if (player)
    {
        int x = D_WIN_MAX_X - 230;
        int y = 80;

        DrawStringToHandle(x + 10, y + 4, "LIFE - STOCK", GetColor(0, 255, 255), font_orbitron);

        for (int i = 0; i < player->GetLife(); ++i)
        {
            int px = x + 20 + i * 20;
            int py = y + 32;
            int sz = 14;
            DrawTriangle(px, py + sz, px + sz / 2, py, px + sz, py + sz, GetColor(255, 100, 100), TRUE);
        }

        // 横幅いっぱいのライン
        DrawLine(x, y + 64, x + 200, y + 64, GetColor(0, 255, 255));
    }

    // ==== CHARGE ゲージ ====
    if (player)
    {
        int x = D_WIN_MAX_X - 230;
        int y = 150;

        DrawStringToHandle(x + 10, y + 2, "CHARGE", GetColor(0, 255, 255), font_orbitron);

        float rate = player->GetChargeRate();
        int bar_x = x + 10, bar_y = y + 25;
        int bar_w = 180, bar_h = 12;

        int fill = player->CanUseSpecial() ? GetColor(0, (GetNowCount() % 100 > 50) ? 255 : 100, 255) : GetColor(0, 255, 255);
        DrawBox(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, GetColor(30, 30, 30), TRUE);
        DrawBox(bar_x, bar_y, bar_x + static_cast<int>(bar_w * rate), bar_y + bar_h, fill, TRUE);

        // "Press B!!" 表示
        if (player->CanUseSpecial())
        {
            int text_x = x + 140;
            int text_y = y + 30;
            int pulse = (GetNowCount() % 100 > 50) ? 255 : 100;
            DrawStringToHandle(text_x - 25, text_y + 5, "Press B!!", GetColor(255, pulse, pulse), font_orbitron);
        }

        DrawLine(x, y + 60, x + 200, y + 60, GetColor(0, 255, 255));
    }

    // ==== POWER（ゲージ型） ====
    if (player)
    {
        int x = D_WIN_MAX_X - 230;
        int y = 220;

        DrawStringToHandle(x + 10, y + 2, "POWER", GetColor(0, 255, 255), font_orbitron);

        int level = player->GetPowerd();
        for (int i = 0; i < 3; ++i)
        {
            int px = x + 20 + i * 20;
            int py = y + 25;
            int col = (i < level) ? GetColor(255, 255, 100) : GetColor(50, 50, 50);
            DrawBox(px, py, px + 14, py + 14, col, TRUE);
        }

        DrawLine(x, y + 60, x + 200, y + 60, GetColor(0, 255, 255));
    }

    // ==== SHIELD ====
    if (player)
    {
        int x = D_WIN_MAX_X - 230;
        int y = 290;

        DrawStringToHandle(x + 10, y + 2, "SHIELD", GetColor(0, 255, 255), font_orbitron);

        const char* shield_text = player->GetShieldOn() ? "ON" : "OFF";
        int color = player->GetShieldOn() ? GetColor(0, 255, 100) : GetColor(255, 100, 100);
        DrawStringToHandle(x + 20, y + 25, shield_text, color, font_digital);

        if (player->GetShieldOn())
        {
            float r = sinf(GetNowCount() / 30.0f) * 5 + 15;
            DrawCircle(x + 150, y + 35, static_cast<int>(r), GetColor(0, 255, 180), FALSE);
        }

        // ラインの位置を少し下にずらす（重なり防止）
        DrawLine(x, y + 70, x + 200, y + 70, GetColor(0, 255, 255));
    }

    // ==== 必殺技ゲージ（LIFEの下、点滅＆レイアウト調整済） ====
    if (player)
    {
        float rate = player->GetChargeRate(); // 0.0 ～ 1.0
        int gauge_x = D_WIN_MAX_X - 230;
        int gauge_y = 150;
        int gauge_w = 200;
        int gauge_h = 50;

        DrawStringToHandle(gauge_x + 10, gauge_y + 2, "CHARGE", GetColor(0, 255, 255), font_orbitron);

        int bar_x = gauge_x + 10;
        int bar_y = gauge_y + 25;
        int bar_w = gauge_w - 20;
        int bar_h = 12;

        int fill_color = player->CanUseSpecial()
            ? GetColor(0, (GetNowCount() % 100 > 50) ? 255 : 100, 255)  // 点滅
            : GetColor(0, 255, 255); // 通常色

        DrawBox(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, GetColor(30, 30, 30), TRUE); // 背景
        DrawBox(bar_x, bar_y, bar_x + static_cast<int>(bar_w * rate), bar_y + bar_h, fill_color, TRUE); // 本体

        if (player->CanUseSpecial())
        {
            for (int i = 0; i < 5; ++i)
            {
                int x1 = bar_x + GetRand(bar_w);
                int x2 = x1 + GetRand(10);
                DrawLine(x1, bar_y, x2, bar_y + bar_h, GetColor(100, 255, 255));
            }
        }
    }

    // ==== SPECIAL READY UI（プレイヤー下に真ん中表示） ====
    if (player && player->CanUseSpecial())
    {

        Vector2D pos = player->GetLocation();
        int ui_x = static_cast<int>(pos.x) - 55;
        int ui_y = static_cast<int>(pos.y) + 40;
        int pulse = static_cast<int>(GetNowCount() % 100) > 50 ? 255 : 100;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawFormatStringToHandle(ui_x, ui_y, GetColor(255, pulse, pulse), font_orbitron, "Press B!!");
    }

    // === 操作UI（右下） ===
    {
        const int base_x = D_WIN_MAX_X - 250;
        const int base_y = D_WIN_MAX_Y - 240;
        const int line_height = 42;

        const char* inputs[] = {
            "MOVE    : LeftStick",
            "SHOT    : A",
            "SPECIAL : B",
            "PAUSE   : Start"
        };

        int pulse = (GetNowCount() % 100 < 50) ? 255 : 100;
        int op_color = GetColor(100, pulse, 255);  // 操作ガイドカラー

        for (int i = 0; i < 4; ++i)
        {
            int y = base_y + i * line_height;
            DrawStringToHandle(base_x, y, inputs[i], op_color, font_orbitron);
        }
    }

    // ==== サイドパネルとの境界に「流れるライン」エフェクト（Stage4は赤?黄のチカチカ） ====
    {
        const int line_width = 4;
        const int flow_speed = 1000;
        const int gap = 70;

        int offset = static_cast<int>(line_effect_timer * flow_speed) % gap;

        // Stage4判定
        bool is_stage4 = (current_stage && current_stage->GetStageID() == StageID::Stage4);

        int pulse_timer = (GetNowCount() / 10) % 2;  // 交互に点滅（10フレームごと）
        int line_color = is_stage4
            ? (pulse_timer == 0 ? GetColor(255, 80, 50) : GetColor(255, 200, 50)) // 赤と黄色
            : GetColor(0, 180, 255); // 通常：ネオンブルー

        int flare_color = is_stage4
            ? (pulse_timer == 0 ? GetColor(255, 120, 120) : GetColor(255, 240, 120)) // フレアも切り替え
            : GetColor(100, 255, 255);

        int left_x = (D_WIN_MAX_X / 2) - 350;
        int right_x = (D_WIN_MAX_X / 2) + 350 - line_width;

        for (int y = -gap; y < D_WIN_MAX_Y + gap; y += gap)
        {
            int y0 = y + offset;

            // 左ライン
            DrawBox(left_x, y0, left_x + line_width, y0 + 20, line_color, TRUE);
            if (is_stage4)
            {
                DrawCircle(left_x + line_width / 2, y0 + 10, 3, flare_color, TRUE); // フレア追加
            }

            // 右ライン
            DrawBox(right_x, y0, right_x + line_width, y0 + 20, line_color, TRUE);
            if (is_stage4)
            {
                DrawCircle(right_x + line_width / 2, y0 + 10, 3, flare_color, TRUE);
            }
        }
    }

    if (isPaused)
    {
        const int cx = D_WIN_MAX_X / 2;
        const int cy = D_WIN_MAX_Y / 2 - 20;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        //DrawBox(400, 200, 880, D_WIN_MAX_Y - 200, GetColor(255, 255, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


        const char* menuItems[] = {
            "START GAME",
            "BACK TITLE",
        };

        for (int i = 0; i < 2; ++i)
        {
            int y = 300 + i * 50;
            int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
            int x = (D_WIN_MAX_X - textWidth) / 2;

            if (i == m_selectedIndex)
            {
                // =========================
                // 背景ハイライトバー（画面端まで）
                // =========================
                int barHeight = 40;
                int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50); // パルス
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
                DrawBox(
                    0, y - 5,
                    D_WIN_MAX_X, y + barHeight,
                    GetColor(0, 200, 255), TRUE
                );

                // =========================
                // 両端のエッジエフェクト（流れる光）
                // =========================
                int edgeWidth = 80;
                int edgeHeight = 4;
                int scrollSpeed = 2;
                int edgeOffset = (GetNowCount() * scrollSpeed) % (D_WIN_MAX_X + edgeWidth * 2);

                // 左から右へ流れる（2個表示してループ感を出す）
                for (int j = 0; j < 2; ++j)
                {
                    int edgeX = edgeOffset - edgeWidth * j;
                    SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
                    DrawBox(
                        edgeX, y + 10,
                        edgeX + edgeWidth, y + 10 + edgeHeight,
                        GetColor(200, 255, 255), TRUE
                    );
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                }

                // =========================
                // テキスト（アウトライン＋グロー）
                // =========================
                int offsetX = (rand() % 3) - 1;
                int offsetY = (rand() % 3) - 1;

                DrawStringToHandle(x + offsetX - 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX + 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX, y + offsetY - 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX, y + offsetY + 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);

                int glow = (int)((sinf(GetNowCount() / 30.0f) + 1.0f) * 127);
                DrawStringToHandle(x + offsetX, y + offsetY, menuItems[i], GetColor(100 + glow, 255, 255), m_menuFontHandle);
            }
            else
            {
                DrawStringToHandle(x, y, menuItems[i], GetColor(180, 180, 180), m_menuFontHandle);
            }
        }
    }

    if (effect_shield_on || effect_shield_off || effect_powerup)
    {
        int alpha = static_cast<int>(100 * (1.0f - (effect_timer / effect_duration))); // 最大100に抑える

        int color = GetColor(0, 0, 0); // デフォルト（透明黒）

        if (effect_shield_on)
            color = GetColor(0, 255, 180); // 緑系（シールドON）
        else if (effect_shield_off)
            color = GetColor(255, 50, 50); // 赤系（シールド破壊）
        else if (effect_powerup)
            color = GetColor(255, 255, 100); // 黄系（パワーアップ）

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

// 警告演出の更新処理
void GameMainScene::WarningUpdate(float delta_second)
{
    if (warning_state == WarningState::None && is_none == false)
    {
        // 条件に応じて（例：一定時間経過後など）
        if (black_in_timer2 >= 2.0f)
        {
            is_none = true;
            warning_state = WarningState::Expanding;
            warning_timer = 0.0f;
            band_half_height = 0.0f; // 念のため初期化
            warning_text_x = 1020;   // 文字位置初期化（右端から流す場合）

            if (player)
                player->SetShotStop(true);
        }

    }

    black_in_timer2 += delta_second;
    warning_timer += delta_second;

    switch (warning_state)
    {
    case WarningState::Expanding:
        band_half_height += band_expand_speed * delta_second;
        if (band_half_height >= band_max_half_height)
        {
            band_half_height = band_max_half_height;
            warning_state = WarningState::Displaying;
            warning_timer = 0.0f;

            // 警告SE再生
            PlaySoundMem(se_warning, DX_PLAYTYPE_BACK);
        }
        break;

    case WarningState::Displaying:
        warning_text_x -= warning_scroll_speed * delta_second;
        warning_timer += delta_second;

        if (warning_text_x < -200)
            warning_text_x = 1020;

        if (warning_timer >= warning_duration)
        {
            warning_state = WarningState::Shrinking;
        }
        break;

    case WarningState::Shrinking:
        band_half_height -= band_expand_speed * delta_second;
        if (band_half_height <= 0.0f)
        {
            band_half_height = 0.0f;
            warning_state = WarningState::None;
        }
        // ステージ4のBGM再生
        PlaySoundMem(stage_bgm4, DX_PLAYTYPE_LOOP);
        if (player)
            player->SetShotStop(false);

        break;

    default:
        break;
    }
}
