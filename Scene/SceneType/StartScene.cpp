#include "StartScene.h"

StartScene::StartScene()
{
}

StartScene::~StartScene()
{
}

// 初期化処理
void StartScene::Initialize()
{
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    m_seTypeSound = rm->GetSounds("Resource/sound/se/se_effect/tap.mp3");
    movie_handle = LoadGraph("Resource/pv/pv.mp4");

    m_nextCharDelay = 0.08f + (GetRand(120) / 1000.0f);  // 0.08～0.2秒

    m_step = Step::WaitForInput;
    m_elapsedTime = 0.0f;
    m_displayCharCount = 0;

    m_fontDigital = CreateFontToHandle("DS-Digital", 28, 6, DX_FONTTYPE_ANTIALIASING);
    m_fontJP = CreateFontToHandle("メイリオ", 22, 6, DX_FONTTYPE_ANTIALIASING);

    m_introText[0] = "これはプロトタイプAIに託された任務―― 記憶を失ったデジタル世界が崩壊の危機に瀕している。";
    m_introText[1] = "侵蝕するウイルスを排除し、秩序を取り戻せ。君の戦いが、再起動の鍵となる。";

    m_idleTimer = 0.0f;
    m_demoPlaying = false;
}

/// 更新処理
eSceneType StartScene::Update(float delta_second)
{
    InputManager* input = Singleton<InputManager>::GetInstance();
    m_elapsedTime += delta_second;

    bool inputDetected =
        input->GetKeyDown(KEY_INPUT_A) || input->GetKeyDown(KEY_INPUT_Z) || input->GetKeyDown(KEY_INPUT_SPACE) ||
        input->GetButtonDown(XINPUT_BUTTON_A);

    // 動画再生中にボタンが押されたら停止して戻る
    if (m_demoPlaying && inputDetected) {
        PauseMovieToGraph(movie_handle);
        SeekMovieToGraph(movie_handle, 0);
        m_demoPlaying = false;
        m_step = Step::WaitForInput;
        m_idleTimer = 0.0f;
        m_elapsedTime = 0.0f;
        return GetNowSceneType();
    }

    // 入力がなければアイドルタイマーを進める（WaitForInput中のみ）
    if (m_step == Step::WaitForInput) {
        if (!inputDetected) {
            m_idleTimer += delta_second;

            if (m_idleTimer > 4.0f) {
                m_step = Step::DemoMovie;
                m_demoPlaying = true;
                PlayMovieToGraph(movie_handle, DX_PLAYTYPE_NORMAL);
            }
        }
        else {
            m_idleTimer = 0.0f; // ★ 入力があればリセット！
        }
    }

    switch (m_step)
    {
    case Step::WaitForInput:
        if (inputDetected) {
            m_idleTimer = 0.0f; // ← ここでも明示的にリセットしておくと安全
            m_step = Step::IntroText;
            m_elapsedTime = 0.0f;
            m_displayCharCount = 0;
        }
        break;

    case Step::IntroText:
    {
        int totalLength = (int)(std::strlen(m_introText[0]) + std::strlen(m_introText[1]));
        if (m_displayCharCount < totalLength) {
            if (m_elapsedTime > m_nextCharDelay) {
                PlaySoundMem(m_seTypeSound, DX_PLAYTYPE_BACK);
                m_displayCharCount++;
                m_elapsedTime = 0.0f;
                m_nextCharDelay = 0.08f + (GetRand(120) / 1000.0f);
            }
        }
        else {
            m_step = Step::GoToTitle;
            m_elapsedTime = 0.0f;
        }

        if (inputDetected) {
            m_displayCharCount = totalLength;
            m_step = Step::GoToTitle;
            m_elapsedTime = 0.0f;
        }
        break;
    }

    case Step::GoToTitle:
        if (m_elapsedTime > 10.0f || inputDetected) {
            return eSceneType::eTitle;
        }
        break;

    case Step::DemoMovie:
        if (GetMovieStateToGraph(movie_handle) == 0) {
            // 一度消して読み直す
            if (movie_handle != -1) {
                DeleteGraph(movie_handle);
                movie_handle = -1;
            }

            movie_handle = LoadGraph("Resource/pv/pv.mp4");

            m_demoPlaying = false;
            m_step = Step::WaitForInput;
            m_idleTimer = 0.0f;
            m_elapsedTime = 0.0f;
        }
        break;
    }

    return GetNowSceneType();
}

/// 描画処理
void StartScene::Draw()
{
#if _DEBUG
    DrawString(0, 0, "StartScene", GetColor(255, 255, 255));
#endif

    if (m_step == Step::DemoMovie) {
        DrawExtendGraph(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, movie_handle, FALSE);
        return;
    }

    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(10, 10, 30), TRUE);

    switch (m_step)
    {
    case Step::WaitForInput:
        if ((int)(m_elapsedTime * 2) % 2 == 0) {
            const char* msg = "PRESS ANY BUTTON";
            int msgWidth = GetDrawStringWidthToHandle(msg, strlen(msg), m_fontDigital);
            int x = (D_WIN_MAX_X - msgWidth) / 2;
            DrawStringToHandle(x, 400, msg, GetColor(255, 255, 255), m_fontJP);
        }
        break;

    case Step::IntroText:
    case Step::GoToTitle:
    {
        int remain = m_displayCharCount;
        for (int i = 0; i < LINE_COUNT; ++i) {
            int lineLen = (int)std::strlen(m_introText[i]);
            int drawLen = remain > lineLen ? lineLen : remain;

            if (drawLen > 0) {
                char buffer[256] = { 0 };
                strncpy_s(buffer, sizeof(buffer), m_introText[i], drawLen);
                buffer[drawLen] = '\0';

                int textWidth = GetDrawStringWidthToHandle(buffer, strlen(buffer), m_fontJP);
                int offsetX = (rand() % 3) - 1;
                int x = (D_WIN_MAX_X - textWidth) / 2 + offsetX;
                int y = 380 + i * 40;
                DrawStringToHandle(x, y, buffer, GetColor(255, 255, 255), m_fontJP);
            }

            remain -= drawLen;
            if (remain <= 0) break;
        }
        break;
    }
    }
}

/// 終了処理
void StartScene::Finalize()
{
    if (m_fontDigital != -1) {
        DeleteFontToHandle(m_fontDigital);
        m_fontDigital = -1;
    }

    if (m_fontJP != -1) {
        DeleteFontToHandle(m_fontJP);
        m_fontJP = -1;
    }

    if (m_seTypeSound != -1) {
        DeleteSoundMem(m_seTypeSound);
        m_seTypeSound = -1;
    }

    if (movie_handle != -1) {
        DeleteGraph(movie_handle);
        movie_handle = -1;
    }

    if (movie_handle != -1) {
        DeleteGraph(movie_handle);
        movie_handle = -1;
    }
}

/// 現在のシーンタイプを返す
eSceneType StartScene::GetNowSceneType() const
{
    return eSceneType::eStart;
}