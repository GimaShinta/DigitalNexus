#pragma once
#include "../SceneBase.h"
#include "../../Utility/ScoreData.h"

class RankingScene : public SceneBase
{
private:
	std::vector<ScoreEntry> entries;
	float total_score = 0.0f;
	int wait_timer = 0;
	int start_time = 0;
	bool confirm_reset = false;
	int cursor_index = 0;  // �J�[�\���ʒu�̒ǉ�

	bool reset_command[4] = { false };

public:
	RankingScene();
	virtual ~RankingScene();

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
	float Max(float a, float b)
	{
		return (a > b) ? a : b;
	}

	bool LoadRankingFromFile();
	void SaveRankingToFile();
	void ResetRankingFile();
	std::string GetTodayString();
	void DrawNeonText(int x, int y, const char* text, int baseColor, int glowColor, int glowPulse);
};

