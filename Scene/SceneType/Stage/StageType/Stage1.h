#pragma once
#include "../StageBase.h"
class Stage1 :
    public StageBase
{
private:
	// �e��U���E�G�L�����N�^�[�̃|�C���^��`
	class Zako* zako = nullptr;             // �G���G����1
	class EnemyShot1* e_shot1 = nullptr;      // �G�̃V���b�g�^�C�v1
	class Boss1* boss = nullptr;

	float enemy_spawn_timer = 0.0f;     // �G�o���^�C�}�[
	bool zako_spawned = false;         // Zako�����łɏo���������ǂ���

	int spawn_index = 0;
	float spawn_delay_timer = 0.0f;
	bool spawning = false;

	bool spawned_enemy1 = false;  // Enemy1�o���t���O

	int font_orbitron;
	int font_warning;

	// ���E�o���Ǘ�
	int spawn_index_left = 0;
	float spawn_delay_timer_left = 0.0f;
	bool spawning_left = false;

	int spawn_index_right = 0;
	float spawn_delay_timer_right = 0.0f;
	bool spawning_right = false;

	// Zako2/Zako7��p��Wave�Ǘ�
	bool zako2_wave_started = false;
	bool zako7_wave_started = false;

private:
	// �����̃����o�̉��ɒǉ�
	bool entry_effect_playing = false;
	float entry_effect_timer = 0.0f;

	bool debug_boss_only = true; // �� �f�o�b�O�p: �{�X�����o��


public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Stage1(Player* player);     // �v���C���[���������Ɏ��X�e�[�W������
	~Stage1();                  // �f�X�g���N�^

	// ��{�I�ȃX�e�[�W�����Q
	void Initialize() override;              // �X�e�[�W����������
	void Finalize() override;                // �X�e�[�W�I������
	void Update(float delta_second) override;       // ���t���[���̍X�V����
	void Draw() override;                    // �`�揈��
	StageBase* GetNextStage(Player* player) override; // ���̃X�e�[�W���擾

private:
	void DrawScrollBackground() const override; // �w�i�̃X�N���[���`��
	void EnemyAppearance(float delta_second) override;
	void UpdateGameStatus(float delta_second) override;
	void StageLabel() const override;
	void AppearancePlayer(float delta_second);


};

