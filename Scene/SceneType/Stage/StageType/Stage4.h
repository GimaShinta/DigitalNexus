#pragma once
#include "../StageBase.h"
class Stage4 :
	public StageBase
{
private:
	// �e��U���E�G�L�����N�^�[�̃|�C���^��`
	class Zako* zako = nullptr;             // �G���G����1
	class Boss4* boss = nullptr;
	class EnemyShot1* e_shot1 = nullptr;      // �G�̃V���b�g�^�C�v1

	bool boss4_spawned = false;
	bool draw_animation_first = false;  // �f�t�H���g��false

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Stage4(Player* player);     // �v���C���[���������Ɏ��X�e�[�W������
	~Stage4();                  // �f�X�g���N�^

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

