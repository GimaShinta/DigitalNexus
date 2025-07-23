#pragma once
#include "../SceneBase.h"
#include "Stage/StageBase.h"
#include <random>

#define UI_CHANGE 0

class GameMainScene : public SceneBase
{
private:
	class Player* player = nullptr;
	StageBase* current_stage;
	float next_scene_timer = 0.0f;
	bool isPaused = false;
private:
	int font_digital;
	int font_orbitron;
	int obi_handle;

	float background_scroll_x = 0.0f;
private:
	int current_bgm_handle = -1;
	int stage_bgm1 = -1; // �X�e�[�W1,2�̋���BGM
	int stage_bgm3 = -1; // �X�e�[�W3�pBGM
	int stage_bgm4 = -1;
	int se_warning = -1;

	float stage3_bgm_volume = 255.0f;
	bool stage3_bgm_fadeout = false;
	bool stage4_bgm_pending = false;
	float se_warning_timer = 0.0f;


	float pause_timer = 0.0f;
	int transparent = 0;

	int se_charge = NULL;
	int cursor_se = NULL;

	// �� �X�R�A���O�����p�\���̂ƕϐ�
	struct ScoreLog
	{
		std::string text;
		float timer;
		float y_offset;
	};

	std::vector<ScoreLog> score_logs;
	float previous_score_count = 0.0f;

	int m_selectedIndex = 0; // �I�𒆂̃C���f�b�N�X
	bool m_startTransitioning = false;  // �o�����o���t���O
	float m_transitionTimer = 0.0f;     // �o�����o�^�C�}�[
	int m_menuFontHandle;
	float line_effect_timer = 0.0f; // ���C�����o�p�̓Ǝ��^�C�}�[

	// --- �t�B���^�[���o�p ---
	bool effect_shield_on = false;
	bool effect_shield_off = false;
	bool effect_powerup = false;
	float effect_timer = 0.0f;
	const float effect_duration = 0.4f;



	int vo = 70;


	bool black_fade_started = false;
	float black_fade_timer = 0.0f;
	float black_in_timer = 0.0f;
	float black_in_timer2 = 0.0f;
	int alpha = 0;
	// �����o�ϐ��Ȃ�
	float band_center_y = 360;          // ��ʒ����̊�ʒu
	float band_half_height = 0.0f;      // �т̔����̍����i0����ő�܂ŕω��j
	const float band_max_half_height = 60.0f; // �т̍ő唼���̍����i�܂荂��120�j
	float band_expand_speed = 200.0f;   // �т��L���鑬�x(px/s)
	int font_warning;
	float warning_text_x = 1280.0f; // �����͉E�[�O
	float warning_timer = 0.0f;
	float warning_scroll_speed = 700.0f; // px/�b
	float warning_duration = 11.0f;

	bool is_none = false;

	enum class WarningState {
		None,       // ����
		Expanding,  // �т��L�����Ă���
		Displaying, // �\�����i�e�L�X�g�X�N���[���j
		Shrinking   // �т��k��
	};

	WarningState warning_state = WarningState::None;

	bool end_sequence_started = false;      // �I�����o���n�܂�����
	float end_sequence_timer = 0.0f;        // �I�����o�^�C�}�[
	int end_fade_alpha = 0;                 // �t�F�[�h�A�E�g�p�A���t�@

	// �O���[�o�� or �����o�ϐ��Ƃ��ă^�C�}�[��p��
	float red_alpha_timer = 0.0f;

	float gameover_alp_timer = 0.0f;
	float gameover_timer = 0.0f;
	int gameover_alpha = 0; // �ԃt�B���^�[�������̃A���t�@
	int gameover_text_alpha = 0; // �ԃt�B���^�[�������̃A���t�@

	float scene_timer = 0.0f;
	bool retry = false;

	PlayerType ui_current_type = PlayerType::AlphaCode;
	PlayerType ui_target_type = PlayerType::AlphaCode;

	float ui_transition_timer = 0.0f;
	bool ui_transitioning = false;
	const float UI_TRANSITION_DURATION = 0.3f; // �b��

public:
	GameMainScene();
	virtual ~GameMainScene();

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

	// �X�R�A���O�ǉ��֐�
	void AddScoreLog(const std::string& text);

	// UI�̕`��i�S���j
	void DrawUI();

	// �x�����o�̍X�V����
	void WarningUpdate(float delta_second);

	void DrawGameOverEffect();

	eSceneType UpdateGameplay(float delta);
	eSceneType ProceedToNextStage(float delta);
	eSceneType UpdateGameOverState(float delta);

	eSceneType UpdatePauseMenu(float delta);

	void StartUITransition(PlayerType new_type);
	void UpdateUITransition(float delta);


	template <typename T>
	T Clamp(T value, T min, T max) {
		return (value < min) ? min : (value > max) ? max : value;
	}

	float Min(float a, float b)
	{
		return (a < b) ? a : b;
	}

	float Lerp(float a, float b, float t) { return a + (b - a) * t; }

#if UI_CHANGE
	int GetTypeColor(int r_alpha, int g_alpha, int b_alpha, int r_omega, int g_omega, int b_omega)
	{
		float t = (ui_transitioning ? ui_transition_timer / UI_TRANSITION_DURATION : 1.0f);
		if (ui_current_type == PlayerType::AlphaCode && ui_target_type == PlayerType::OmegaCode)
		{
			return GetColor(
				(int)Lerp(r_alpha, r_omega, t),
				(int)Lerp(g_alpha, g_omega, t),
				(int)Lerp(b_alpha, b_omega, t)
			);
		}
		else if (ui_current_type == PlayerType::OmegaCode && ui_target_type == PlayerType::AlphaCode)
		{
			return GetColor(
				(int)Lerp(r_omega, r_alpha, t),
				(int)Lerp(g_omega, g_alpha, t),
				(int)Lerp(b_omega, b_alpha, t)
			);
		}
		else if (ui_current_type == PlayerType::OmegaCode)
		{
			return GetColor(r_omega, g_omega, b_omega);
		}
		else
		{
			return GetColor(r_alpha, g_alpha, b_alpha);
		}
	}

	int GetUIXOffset()
	{
		if (!ui_transitioning) return 0;

		float t = ui_transition_timer / UI_TRANSITION_DURATION;
		if (t < 0.5f)
			return static_cast<int>(Lerp(0, -D_WIN_MAX_X, t * 2)); // ���֑ޔ�
		else
			return static_cast<int>(Lerp(D_WIN_MAX_X, 0, (t - 0.5f) * 2)); // �E����߂�
	}

	int GetUIXOffsetLeft()
	{
		if (!ui_transitioning) return 0;

		float t = ui_transition_timer / UI_TRANSITION_DURATION;
		if (t < 0.5f)
		{
			// ���֑ޔ�
			return static_cast<int>(Lerp(0, -D_WIN_MAX_X, t * 2));
		}
		else
		{
			// ���̈ʒu�ցi�E����߂�j
			return static_cast<int>(-Lerp(D_WIN_MAX_X, 0, (t - 0.5f) * 2));
		}
	}

	int GetUIXOffsetRight()
	{
		if (!ui_transitioning) return 0;

		float t = ui_transition_timer / UI_TRANSITION_DURATION;
		if (t < 0.5f)
		{
			// �E�֑ޔ�
			return static_cast<int>(Lerp(0, D_WIN_MAX_X, t * 2));
		}
		else
		{
			// ���̈ʒu�ցi������߂�j
			return static_cast<int>(-Lerp(-D_WIN_MAX_X, 0, (t - 0.5f) * 2));
		}
	}

#else

	int GetNoiseOffset(float strength)
	{
		static std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(-strength, strength);
		return static_cast<int>(dist(rng));
		return 0;
	}

	int GetUIXOffsetLeft()
	{
		if (!ui_transitioning) return 0;

		float t = ui_transition_timer / UI_TRANSITION_DURATION;
		float strength = (1.0f - fabs(1.0f - t * 2)) * 6.0f; // ���Ԃōł��W���W��
		return 0;
	}

	int GetUIXOffsetRight()
	{
		if (!ui_transitioning) return 0;

		float t = ui_transition_timer / UI_TRANSITION_DURATION;
		float strength = (1.0f - fabs(1.0f - t * 2)) * 6.0f;
		return 0;
	}

	int GetTypeColor(int r_alpha, int g_alpha, int b_alpha, int r_omega, int g_omega, int b_omega)
	{
		float t = (ui_transitioning ? ui_transition_timer / UI_TRANSITION_DURATION : 1.0f);

		if (ui_transitioning)
		{
			// �Â߂ŕs����ȐF�Ƀt���b�V���i��ʓx�E�����ݐF�j
			int flash_state = static_cast<int>(t * 20.0f) % 4;
			switch (flash_state)
			{
			case 0: return GetColor(60, 60, 60);     // �ÃO���[
			case 1: return GetColor(80, 40, 40);     // �Ð�
			case 2: return GetColor(40, 80, 40);     // �×�
			case 3: return GetColor(40, 40, 80);     // �Ð�
			}
		}

		// �ʏ�̕�ԐF
		if (ui_current_type == PlayerType::AlphaCode && ui_target_type == PlayerType::OmegaCode)
		{
			return GetColor(
				(int)Lerp(r_alpha, r_omega, t),
				(int)Lerp(g_alpha, g_omega, t),
				(int)Lerp(b_alpha, b_omega, t)
			);
		}
		else if (ui_current_type == PlayerType::OmegaCode && ui_target_type == PlayerType::AlphaCode)
		{
			return GetColor(
				(int)Lerp(r_omega, r_alpha, t),
				(int)Lerp(g_omega, g_alpha, t),
				(int)Lerp(b_omega, b_alpha, t)
			);
		}
		else if (ui_current_type == PlayerType::OmegaCode)
		{
			return GetColor(r_omega, g_omega, b_omega);
		}
		else
		{
			return GetColor(r_alpha, g_alpha, b_alpha);
		}
	}
#endif

	void DrawLineNoiseEffectForSideUI(float intensity = 1.0f)
	{
		const int lineCount = static_cast<int>(50 * intensity);
		const int uiWidth = 290;
		const int screenWidth = D_WIN_MAX_X;
		const int noiseOverflow = 50; // UI����͂ݏo����

		for (int i = 0; i < lineCount; ++i)
		{
			int y = GetRand(D_WIN_MAX_Y);
			int height = GetRand(2) + 5;
			int width = GetRand(100) + 50;
			int brightness = 30 + GetRand(60);

			bool isLeft = (GetRand(1) == 0);

			int x;
			if (isLeft)
			{
				// ��UI: x = -20 �` 290
				x = -noiseOverflow + GetRand(uiWidth + noiseOverflow);
				if (x + width > uiWidth + noiseOverflow) width = (uiWidth + noiseOverflow) - x;
			}
			else
			{
				// �EUI: x = 990 - 20 �` 1280
				const int rightStart = screenWidth - uiWidth;
				x = rightStart - noiseOverflow + GetRand(uiWidth + noiseOverflow);
				if (x + width > screenWidth) width = screenWidth - x;
			}

			DrawBox(x, y, x + width, y + height, GetColor(brightness, brightness, brightness), TRUE);
		}
	}
};