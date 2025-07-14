#pragma once
#include "../SceneBase.h"
#include <vector>
class TitleScene : public SceneBase
{
private:
	int m_selectedIndex = 0; // �I�𒆂̃C���f�b�N�X
	int m_menuFontHandle = 0;

	int m_logoHandle = 0;
	float m_logoScale = 0.0f; // �A�j���p�X�P�[��
	int m_logoAlpha = 0;   // �A�j���p�����x

	//���S�̍��W�����ϐ�
	int m_logoX = 0;
	int m_logoY = 0;
	int m_logoW = 0;
	int m_logoH = 0;

	float m_logoAppearTimer = 0.0f;
	bool m_logoAppeared = false;

	int logo_se = NULL;
	int bgm = NULL;
	int cursor_se = NULL;
	int tap_se = NULL;

	int drawY = 0;
	float m_cursorY = 0.0f; // �J�[�\��Y���W



	//�I������ԃt���O
	bool m_startTransitioning = false;  // �o�����o���t���O
	float m_transitionTimer = 0.0f;     // �o�����o�^�C�}�[


	float m_menuScale = 1.0f;
	float m_menuAlpha = 255.0f;


	//�p�[�e�B�N���̉��o�p
	struct Particle {
		float x, y, vy;
		int alpha;
	};
	std::vector<Particle> m_particles;

	float exit_scene_timer = 0.0f;
	bool exit_scene = false;
	float exit_trans_timer = 0.0f;
	int exit_trans = 0;
	int text_alpha = 0;     // 0?255 �ŕ����̖��邳
	bool on_text = false;
public:
	TitleScene();
	virtual ~TitleScene();

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

private:
	//�`��n�֐�
	void DrawBackgroundGrid();	//�w�i
	void DrawLogo();			//���S
	void DrawParticles();		//�p�[�e�B�N��
	void DrawMenu();			//���j���[

};

