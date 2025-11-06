#include "TitleScene.h"
#include "../../Utility/ScoreData.h"
 
TitleScene::TitleScene()
{
}

TitleScene::~TitleScene()
{
}

// 初期化処理
void TitleScene::Initialize()
{
    m_menuFontHandle = CreateFontToHandle("Orbitron", 36, 6); // メニュー専用フォント
    m_selectedIndex = 0;
    m_cursorY = 370.0f; // 初期Y座標

    m_startTransitioning = false;
    m_transitionTimer = 0.0f;
    m_menuScale = 1.0f;
    m_menuAlpha = 255.0f;

    m_logoAppearTimer = 0.0f;
    m_logoAppeared = false;
    m_logoScale = 1.3f; // ←控えめな拡大率に
    m_logoAlpha = 0;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    // logo_se = rm->GetSounds("Resource/sound/se/battle/audiostock_1498491.mp3");
    bgm = rm->GetSounds("Resource/sound/bgm/TitleBGM.mp3");
    cursor_se = rm->GetSounds("Resource/sound/se/se_effect/cursor.mp3");
    tap_se = rm->GetSounds("Resource/sound/se/effect/start_se.mp3");

    ChangeVolumeSoundMem(255 * 60 / 100, bgm);
    PlaySoundMem(bgm, DX_PLAYTYPE_LOOP);

    // ロゴ画像読み込み（透過PNG）
    m_logoHandle = rm->GetImages("Resource/Image/BackGround/Title/DigitalNexus.png")[0];

    m_logoScale = 1.5f;   // 拡大から始まる
    m_logoAlpha = 0;      // フェードイン開始

    // ロゴサイズ取得
    GetGraphSize(m_logoHandle, &m_logoW, &m_logoH);
    m_logoScale = 0.7f;
    m_logoAlpha = 0;

    // ロゴ表示位置（中央寄せ）
    m_logoX = (D_WIN_MAX_X - (int)(m_logoW * m_logoScale)) / 2;
    m_logoY = -120;

    // 粒子を初期化（ランダムな位置に配置）
    for (int i = 0; i < 30; ++i)
    {
        Particle p;
        p.x = (float)(rand() % D_WIN_MAX_X);
        p.y = (float)(D_WIN_MAX_Y - rand() % 100); // ← 下からランダム
        p.vy = -0.3f - (rand() % 30) / 100.0f;
        p.alpha = 128 + rand() % 128;
        m_particles.push_back(p);
    }
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType TitleScene::Update(float delta_second)
{
	//InputManager* input = Singleton<InputManager>::GetInstance();

	//if (input->GetKeyDown(KEY_INPUT_SPACE))
	//	return eSceneType::eGameMain;

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

    if (!exit_scene)
    {
        if (movedUp)
        {
            if (m_menuLayer == MenuLayer::MAIN)
            {
                PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
                m_selectedIndex = (m_selectedIndex + 3) % 2;
            }
            else
            {
                PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
                m_selectedIndex = (m_selectedIndex + 3) % 4;
            }
        }
        if (movedDown)
        {
            if (m_menuLayer == MenuLayer::MAIN)
            {
                PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
                m_selectedIndex = (m_selectedIndex + 1) % 2;
            }
            else
            {
                PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
                m_selectedIndex = (m_selectedIndex + 1) % 4;
            }
        }
    }


    // 決定（スペース or Aボタン）
    if (!m_startTransitioning
        && !exit_scene                                      // ★追加：Exit 演出中は決定無効
        && (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A)))
    {
        if (m_menuLayer == MenuLayer::MAIN)
        {
            PlaySoundMem(tap_se, DX_PLAYTYPE_BACK);

            if (m_selectedIndex == 0)
                m_menuLayer = MenuLayer::SUB;
            else
                exit_scene = true;                   // ← 一度 true になったら以降は無効
        }
        else
        {
            PlaySoundMem(tap_se, DX_PLAYTYPE_BACK);

            if (m_selectedIndex == 0) {
                m_startTransitioning = true;
                m_transitionTimer = 0.0f;
            }
            else if (m_selectedIndex == 1) {
                return eSceneType::eRanking;
            }
            else if (m_selectedIndex == 2) {
                return eSceneType::eCredit;
            }
            else if (m_selectedIndex == 3) {
                m_menuLayer = MenuLayer::MAIN;
                m_selectedIndex = 0;
            }
        }
    }


    if (exit_scene == true)
    {
        exit_scene_timer += delta_second;
        exit_trans_timer += delta_second;
        if (exit_trans_timer >= 0.01f)
        {
            exit_trans_timer = 0.0f;
            if (exit_trans < 255)
                exit_trans++;
            else
                text_alpha++;
        }

        if (exit_scene_timer >= 8.0f)
        {
            return eSceneType::eExit;
        }
    }

    // 出撃演出中のタイマー進行
    if (m_startTransitioning)
    {
        StopSoundMem(bgm);

        m_transitionTimer += delta_second;
        ScoreData::GetInstance()->Reset();

        //ScoreData::GetInstance()->Reset();
        if (m_transitionTimer >= 2.0f) // 2秒で次のシーンへ
            return eSceneType::eSelect;
    }


    // ロゴ演出（ゆっくり縮小＆透明度アップ）
    if (!m_logoAppeared)
    {
        m_logoAppearTimer += delta_second;
        float t = m_logoAppearTimer / 1.0f;
        if (t >= 1.0f)
        {
            t = 1.0f;
            m_logoAppeared = true;
        }

        // 1.3 → 0.7 にイージング（緩やかに縮小）
        m_logoScale = 0.7f + (1.3f - 0.7f) * powf(1.0f - t, 2); // EaseOutQuad
        m_logoAlpha = (int)(t * 255.0f);
    }



    // --- カーソル（矢印）のY位置をイージングで追従 ---
    float targetY = 400.0f + m_selectedIndex * 50.0f;
    float diff = targetY - m_cursorY;
    m_cursorY += diff * 0.2f;
    if (fabsf(diff) < 1.0f) m_cursorY = targetY;

    if (m_startTransitioning)
    {
        m_transitionTimer += delta_second;

        // メニュー項目の拡大・消失演出
        m_menuScale += delta_second * 1.0f;   // 拡大
        m_menuAlpha -= delta_second * 300.0f; // フェードアウト
        if (m_menuAlpha < 0.0f) m_menuAlpha = 0.0f;

        if (m_transitionTimer >= 2.0f)
            return eSceneType::eSelect;
    }

    // パーティクルの更新
    for (auto& p : m_particles)
    {
        p.y += p.vy;
        if (p.y < 0) p.y = D_WIN_MAX_Y; // 上に出たら下から出るようにループ
    }

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void TitleScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "TitleScene", GetColor(255, 255, 255));
#endif

    DrawBackgroundGrid();
    DrawLogo();
    DrawParticles();
    DrawMenu();
    if (m_startTransitioning)
    {
        float t = m_transitionTimer / 2.0f;
        if (t > 1.0f) t = 1.0f;

        // イージング（EaseInOutCubic）
        float eased = t < 0.5f
            ? 4 * t * t * t
            : 1 - powf(-2 * t + 2, 3) / 2;

        // スケール：最初は少し拡大→中央に吸い込む
        float scale = 0.7f * (1.0f - eased) + 1.2f * (1.0f - (1.0f - eased) * (1.0f - eased));
        int alpha = (int)((1.0f - eased) * 255);

        int scaledW = (int)(m_logoW * scale);
        int scaledH = (int)(m_logoH * scale);

        int drawX = (D_WIN_MAX_X - scaledW) / 2;
        int drawY = m_logoY + (int)((m_logoH * 0.7f - scaledH) / 2 - 50 * eased);  // 少し上に上がる

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawExtendGraph(drawX, drawY, drawX + scaledW, drawY + scaledH, m_logoHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 残光 or 閃光風の白枠（薄く）
        if (eased < 0.8f) {
            int glowAlpha = (int)((1.0f - eased) * 80);
            SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);
            DrawBox(drawX - 8, drawY - 8, drawX + scaledW + 8, drawY + scaledH + 8, GetColor(180, 240, 255), FALSE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        // 背景の暗転
        int fade = (int)(eased * 255);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, fade);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // CONNECTING 表示
        // ゲージ風 LOADING 演出（1.4秒後に表示）
        if (m_transitionTimer > 1.4f)
        {
            float loadingT = (m_transitionTimer - 1.4f) / 0.6f; // 0?1
            if (loadingT > 1.0f) loadingT = 1.0f;

            int cx = D_WIN_MAX_X / 2;
            int cy = D_WIN_MAX_Y / 2 + 40;

            const int barWidth = 300;
            const int barHeight = 16;
            int filledWidth = (int)(barWidth * loadingT);

            // 枠
            DrawBox(cx - barWidth / 2, cy, cx + barWidth / 2, cy + barHeight, GetColor(100, 100, 100), FALSE);
            // 中身
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
            DrawBox(cx - barWidth / 2, cy, cx - barWidth / 2 + filledWidth, cy + barHeight, GetColor(80, 200, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // LOADING文字
            const char* msg = "LOADING";
            int msgWidth = GetDrawStringWidthToHandle(msg, strlen(msg), m_menuFontHandle);
            DrawStringToHandle(cx - msgWidth / 2, cy - 30, msg, GetColor(200, 255, 255), m_menuFontHandle);
        }
    }

    if (exit_scene)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, exit_trans);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 2. 暗転完了後に文字をフェードイン
        if (exit_trans >= 255)
        {
            SetFontSize(32);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, text_alpha);
            DrawString(D_WIN_MAX_X / 2 - 200, D_WIN_MAX_Y / 2, "Thank You For Playing", GetColor(255, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            SetFontSize(16);
        }
    }
}

// 終了時処理（使ったインスタンスの削除とか）
void TitleScene::Finalize()
{
    if (m_menuFontHandle != -1) {
        DeleteFontToHandle(m_menuFontHandle);
        m_menuFontHandle = -1;
    }
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType TitleScene::GetNowSceneType() const
{
	return eSceneType::eTitle;
}

void TitleScene::DrawBackgroundGrid()
{
	int grid_size = 40;
	int pulseColor = 50 + (int)(sinf(GetNowCount() / 60.0f) * 50);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
	for (int x = 0; x < D_WIN_MAX_X; x += grid_size)
		DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(0, pulseColor, 100));
	for (int y = 0; y < D_WIN_MAX_Y; y += grid_size)
		DrawLine(0, y, D_WIN_MAX_X, y, GetColor(0, pulseColor, 100));
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::DrawLogo()
{
	int scaledW = (int)(m_logoW * m_logoScale);
	int scaledH = (int)(m_logoH * m_logoScale);
	int drawX = (D_WIN_MAX_X - scaledW) / 2;
	int drawY = m_logoY + (int)((m_logoH * 0.7f - scaledH) / 2);

	// 控えめフラッシュ（中央周囲のみ）
	if (!m_logoAppeared)
	{
		//int flashAlpha = (int)((1.0f - m_logoAppearTimer / 1.0f) * 100.0f);
		//SetDrawBlendMode(DX_BLENDMODE_ADD, flashAlpha);
		//DrawBox(drawX - 10, drawY - 10, drawX + scaledW + 10, drawY + scaledH + 10, GetColor(100, 200, 255), TRUE);
		//SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_logoAlpha);
	DrawExtendGraph(drawX, drawY, drawX + scaledW, drawY + scaledH, m_logoHandle, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::DrawParticles()
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	for (const auto& p : m_particles)
	{
		DrawCircle((int)p.x, (int)p.y, 2, GetColor(100, 200, 255), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TitleScene::DrawMenu()
{
    const char** menuItems;  // ★ここが重要：「配列」ではなく「ポインタのポインタ」
    int menuCount = 0;       // メニュー項目数を記録する

    if (m_menuLayer == MenuLayer::MAIN)
    {
        static const char* mainItems[] = {
            "START GAME",
            "EXIT"
        };
        menuItems = mainItems;
        menuCount = 2;
    }
    else
    {
        static const char* subItems[] = {
            "SELECT GAME",
            "RANKING",
            "CREDITS",
            "BACK"
        };
        menuItems = subItems;
        menuCount = 4;
    }

    for (int i = 0; i < menuCount; ++i)
    {
        int y = 400 + i * 50;
        int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
        int x = (D_WIN_MAX_X - textWidth) / 2;

        if (i == m_selectedIndex)
        {
            // =========================
            // 背景ハイライトバー
            // =========================
            int barHeight = 40;
            int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
            DrawBox(0, y - 5, D_WIN_MAX_X, y + barHeight, GetColor(0, 200, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // =========================
            // テキストのグロー・アウトライン
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
