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

    // m_seTypeSound = LoadSoundMem("C:\PG\SAGA_TGS2025\Resource\sound\se\se_effect\tap.mp3"); // 任意のSEファイル

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    m_seTypeSound = rm->GetSounds("Resource/sound/se/se_effect/tap.mp3");

    m_nextCharDelay = 0.08f + (GetRand(120) / 1000.0f);  // → 0.08～0.2秒


    m_step = Step::WaitForInput;
    m_elapsedTime = 0.0f;
    m_displayCharCount = 0;

    m_fontDigital = CreateFontToHandle("DS-Digital", 28, 6, DX_FONTTYPE_ANTIALIASING);
    m_fontJP = CreateFontToHandle("メイリオ", 22, 6, DX_FONTTYPE_ANTIALIASING);


    m_introText[0] = "これはプロトタイプAIに託された任務―― 記憶を失ったデジタル世界が崩壊の危機に瀕している。";
    m_introText[1] = "侵蝕するウイルスを排除し、秩序を取り戻せ。君の戦いが、再起動の鍵となる。";
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType StartScene::Update(float delta_second)
{
	//InputManager* input = Singleton<InputManager>::GetInstance();

	//if (input->GetKeyDown(KEY_INPUT_SPACE))
	//	return eSceneType::eTitle;

    InputManager* input = Singleton<InputManager>::GetInstance();
    m_elapsedTime += delta_second;

    switch (m_step)
    {
    case Step::WaitForInput:
        if (input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_A)) {
            m_step = Step::IntroText;
            m_elapsedTime = 0.0f;
            m_displayCharCount = 0;
        }
        break;

    case Step::IntroText:
    {
        int totalLength = (int)(std::strlen(m_introText[0]) + std::strlen(m_introText[1]));
        if (m_displayCharCount < totalLength)
        {
            if (m_elapsedTime > m_nextCharDelay)
            {
                PlaySoundMem(m_seTypeSound, DX_PLAYTYPE_BACK);
                m_displayCharCount++;
                m_elapsedTime = 0.0f;

                // 次の文字表示までのディレイ（0.08?0.2秒）
                m_nextCharDelay = 0.08f + (GetRand(120) / 1000.0f);
            }
        }

        else {
            m_step = Step::GoToTitle;
            m_elapsedTime = 0.0f;
        }

        if (input->GetKeyDown(KEY_INPUT_Z) || input->GetKeyDown(KEY_INPUT_SPACE) ||
            input->GetButtonDown(XINPUT_BUTTON_A)) {
            m_displayCharCount = totalLength;
            m_step = Step::GoToTitle;
            m_elapsedTime = 0.0f;
        }
        break;
    }

    case Step::GoToTitle:
        if (m_elapsedTime > 2.0f || input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_A)) {
            return eSceneType::eTitle;
        }
        break;
    }

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void StartScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "StartScene", GetColor(255, 255, 255));
#endif

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

// 終了時処理（使ったインスタンスの削除とか）
void StartScene::Finalize()
{
    if (m_fontDigital != -1 &&
        m_fontJP != -1) {
        DeleteFontToHandle(m_fontDigital);
        DeleteFontToHandle(m_fontJP);
        m_fontDigital = -1;
        m_fontJP = -1;

    }

    if (m_seTypeSound != -1) {
        DeleteSoundMem(m_seTypeSound);
        m_seTypeSound = -1;
    }
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType StartScene::GetNowSceneType() const
{
	return eSceneType::eStart;
}
