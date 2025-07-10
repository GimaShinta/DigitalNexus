#pragma once
#include "../Singleton.h"
#include "Vector2D.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>


enum EffectName
{
	eExprotion = 0,
	eExprotion2,
	eChenge,
	eExprotion3,
};

// EffectID �� int�^�Ɠ����ɂ���
using EffectID = int;

class EffectManager : public Singleton<EffectManager>
{
	// �e�G�t�F�N�g�ɍ��ڐݒ�
	struct Effect
	{
		EffectID id;                       // �G�t�F�N�g����ʂ���ID
		std::vector<int> image_handles;    // �G�t�F�N�g�̉摜�i�j�����ł��P�̂ł�
		Vector2D position;                 // �`��ʒu
		float elapsed_time = 0.0f;         // �ݐώ���
		float frame_time = 0.1f;           // �e�t���[�������b�\�����邩
		int current_frame = 0;             // ���ݕ\�����Ă���t���[���ԍ�
		bool loop = false;                 // �G�t�F�N�g�����[�v�����邩
		bool is_paused = false;            // �ꎞ��~�����邩�ǂ���
		bool is_finished = false;          // �I�������邩�ǂ���

		int alpha = 255;                   // �����x
		float scale = 1.0f;  // �g�嗦
		int z_layer = 0;                       // ���C���[

		// ����������
		Effect(EffectID id_, const std::vector<int>& handles, const Vector2D& pos,
			float frame_time_sec, bool loop_flag)
			: id(id_), image_handles(handles), position(pos), frame_time(frame_time_sec), loop(loop_flag) {}
	};

public:
	EffectManager() = default;

	void LoadAllEffects();

	EffectID PlayerAnimation(EffectName effect_name, const Vector2D& position,
		float frame_time_sec, bool loop);

	// �X�V����
	void Update(const float& delta_second);

	// �`�揈��
	void Draw();

	// �I��������
	void Finalize();

public:
	// �w��ID�̃A�j���[�V�������I�����Ă��邩�ǂ�����Ԃ�
	bool GetAnimationFinished(EffectID id) const;

	// �w��ID�̃A�j���[�V�����𑦍폜
	void RemoveAnimation(EffectID id);

	// ���l�̐ݒ�i�O�`�Q�T�T�j
	void SetAlpha(EffectID id, int alpha);

	// �g�嗦�ݒ�i1.0�@���@���{�j
	void SetScale(EffectID id, float scale);

	// �ꎞ��~
	void PauseAnimation(EffectID id);

	// �ĊJ
	void ResumeAnimation(EffectID id);

	// ���C���[�̐ݒ�
	void SetZLayer(EffectID id, int z);

	// �ʒu���̐ݒ�
	void SetPosition(EffectID id, const Vector2D& posi);

private:
	// ���ɍĐ�����ID�ԍ�
	EffectID next_id = 0;

	// ID�ƃA�j���[�V�����̑Ή��}�b�v
	std::unordered_map<EffectID, std::unique_ptr<Effect>> animations;

	std::map<int, std::vector<int>> effect_images;
};

