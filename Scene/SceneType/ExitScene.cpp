#include "ExitScene.h"

ExitScene::ExitScene()
{
}

ExitScene::~ExitScene()
{
}

// ����������
void ExitScene::Initialize()
{
}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="delta_second">�P�t���[��������̎���</param>
/// <returns></returns>
eSceneType ExitScene::Update(float delta_second)
{
	return GetNowSceneType();
}

/// <summary>
/// �`�揈��
/// </summary>
/// <returns></returns>
void ExitScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "ExitScene", GetColor(255, 255, 255));
#endif
}

// �I���������i�g�����C���X�^���X�̍폜�Ƃ��j
void ExitScene::Finalize()
{
}

/// <summary>
/// ���݂̃V�[�����
/// </summary>
/// <returns>���݂̓��U���g�V�[���ł�</returns>
eSceneType ExitScene::GetNowSceneType() const
{
	return eSceneType::eExit;
}
