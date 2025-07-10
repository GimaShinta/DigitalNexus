#include "CreditScene.h"

CreditScene::CreditScene()
{
}

CreditScene::~CreditScene()
{
}

// ����������
void CreditScene::Initialize()
{
    creditLines = {
    "CREDITS",
    "",
    "CharacterDesign: Minakaze Dots",
    "Stage1.2 / BGM: STUDIO incho3",
    "Stage3 / BGM: Tak_mfk",
    "FinalBoss / BGM: maruya328",
    "SE: Audio Stock",
    "",
    "Special Thanks: You!",
    "",
    "Thank you for playing!",
    "",
    "Press A to return..."
    };

    scrollY = D_WIN_MAX_Y;
    scrollSpeed = 30.0f;
}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="delta_second">�P�t���[��������̎���</param>
/// <returns></returns>
eSceneType CreditScene::Update(float delta_second)
{
	//InputManager* input = Singleton<InputManager>::GetInstance();

	//if (input->GetKeyDown(KEY_INPUT_SPACE))
	//	return eSceneType::eExit;

    scrollY -= scrollSpeed * delta_second;

    InputManager* input = Singleton<InputManager>::GetInstance();
    if (input->GetKeyDown(KEY_INPUT_Z) || input->GetButtonDown(XINPUT_BUTTON_A) || scrollY < -static_cast<float>(creditLines.size() * 40))
    {
        return eSceneType::eTitle;
    }

	return GetNowSceneType();
}

/// <summary>
/// �`�揈��
/// </summary>
/// <returns></returns>
void CreditScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "CreditScene", GetColor(255, 255, 255));
#endif

    SetFontSize(28);
    int cx = D_WIN_MAX_X / 2;

    for (size_t i = 0; i < creditLines.size(); ++i)
    {
        int y = static_cast<int>(scrollY + i * 40);
        int text_w = GetDrawStringWidth(creditLines[i], strlen(creditLines[i]));
        DrawString(cx - text_w / 2, y, creditLines[i], GetColor(255, 255, 255));
    }

    SetFontSize(16);
}

// �I���������i�g�����C���X�^���X�̍폜�Ƃ��j
void CreditScene::Finalize()
{
}

/// <summary>
/// ���݂̃V�[�����
/// </summary>
/// <returns>���݂̓��U���g�V�[���ł�</returns>
eSceneType CreditScene::GetNowSceneType() const
{
	return eSceneType::eCredit;
}
