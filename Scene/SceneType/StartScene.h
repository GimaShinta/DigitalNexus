#pragma once
#include "../SceneBase.h"
class StartScene : public SceneBase
{
private:
	enum class Step {
		WaitForInput,
		IntroText,
		GoToTitle,
		DemoMovie,
	};

	Step m_step;
	float m_elapsedTime = 0.0f;
	int m_displayCharCount = 0;
	static const int LINE_COUNT = 2;
	const char* m_introText[LINE_COUNT]; // 2�s���̕�����

	int m_fontDigital = -1;
	int m_fontJP = -1;

	int m_seTypeSound = -1;  // 1��������SE
	float m_nextCharDelay = 0.0f; // ���̕����܂ł̃����_���ҋ@����

	bool m_demoPlaying = false;
	float m_idleTimer = 0.0f;
	int movie_handle = NULL;

public:
	StartScene();
	virtual ~StartScene();

	// ����������
	virtual void Initialize() override;

	/// <summary>
	/// �X�V����
	/// </summary>
	/// <param name="delta_second">�P�t���[��������̎���</param>
	/// <returns></returns>
	virtual eSceneType Update(float delta_second) override;

	/// <summary>
	/// �`�揈��
	/// </summary>
	/// <returns></returns>
	virtual void Draw() override;

	// �I���������i�g�����C���X�^���X�̍폜�Ƃ��j
	virtual void Finalize() override;

	// ���݂̃V�[���^�C�v�i�I�[�o�[���C�h�K�{�j
	virtual eSceneType GetNowSceneType() const override;
};

