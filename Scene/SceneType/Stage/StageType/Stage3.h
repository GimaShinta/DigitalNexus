#pragma once
#include "../StageBase.h"
class Stage3 :
	public StageBase
{
private:
	// �e��U���E�G�L�����N�^�[�̃|�C���^��`
	class Zako* zako = nullptr;             // �G���G����1
	class Boss3* boss = nullptr;
	class EnemyShot1* e_shot1 = nullptr;      // �G�̃V���b�g�^�C�v1

	float enemy_spawn_timer = 0.0f;     // �G�o���^�C�}�[
	float enemy3_spawn_timer = 0.0f;
	bool zako_spawned = false;         // Zako�����łɏo���������ǂ���
	bool boss_spawned = false;

	// ���o���Ǘ�
	int spawn_index_left = 0;
	float spawn_delay_timer_left = 0.0f;
	bool spawning_zako_left = false;

	// �E�o���Ǘ�
	int spawn_index_right = 0;
	float spawn_delay_timer_right = 0.0f;
	bool spawning_zako_right = false;

	bool is_enemy3_group_spawned = false;
	bool draw_animation_first = false;  // �f�t�H���g��false

	//�O���b�h
	mutable float scroll_back = 0.0f;
	mutable float scroll_front = 0.0f;


	float result_timer = 0.0f;
	float total_score = 0.0f;
	bool result_displayed = false;

	float clear_wait_timer = 0.0f;
	bool result_started = false;

	float glitch_timer = 0.0f;
	bool glitch_started = false;
	bool glitch_done = false;
	float post_result_wait_timer = 0.0f;  // �������ǉ�
	float delta_draw = 0.0f;

	bool result_fadeout_started = false;
	float result_fadeout_timer = 0.0f;
	bool result_ended = false;

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Stage3(Player* player);     // �v���C���[���������Ɏ��X�e�[�W������
	~Stage3();                  // �f�X�g���N�^

	// ��{�I�ȃX�e�[�W�����Q
	void Initialize() override;              // �X�e�[�W����������
	void Finalize() override;                // �X�e�[�W�I������
	void Update(float delta_second) override;       // ���t���[���̍X�V����
	void Draw() override;                    // �`�揈��
	StageBase* GetNextStage(Player* player) override; // ���̃X�e�[�W���擾

private:
	void DrawScrollBackground() const override; // �w�i�̃X�N���[���`��
	void ScrollEffectUpdate(float delta_second);
	void DrawFrontGrid() const;
	void EnemyAppearance(float delta_second) override;
	void UpdateGameStatus(float delta_second) override;
	void ResultDraw(float delta_second);

	template <typename T>
	T my_max(const T& a, const T& b)
	{
		return (a > b) ? a : b;
	}
};

