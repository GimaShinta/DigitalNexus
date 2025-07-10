#pragma once
#include "../StageBase.h"
class Stage2 :
	public StageBase
{
private:
	// �e��U���E�G�L�����N�^�[�̃|�C���^��`
	class Zako* zako;             // �G���G����1
	class Boss1* boss;
	class EnemyShot1* e_shot1;      // �G�̃V���b�g�^�C�v1

	float enemy_spawn_timer = 0.0f;     // �G�o���^�C�}�[
	bool zako_spawned = false;         // Zako�����łɏo���������ǂ���

	bool spawned_stair_done = false;
	bool spawned_slash_done = false;
	int stair_index = 0;
	int slash_index = 0;
	float stair_timer = 0.0f;
	float slash_timer = 0.0f;
	//bool stage2boss_spawned = false;
	bool enemy2_spawned = false;

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Stage2(Player* player);     // �v���C���[���������Ɏ��X�e�[�W������
	~Stage2();                  // �f�X�g���N�^

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
};

