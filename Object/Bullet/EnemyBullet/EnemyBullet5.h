#pragma once
#include "EnemyBulletBase.h"
class EnemyBullet5 : public EnemyBulletBase
{
private:
	Vector2D base_velocity = { 0.0f, 0.0f };  // �i�s�����i�ʏ�͐^���Ȃǁj
	float amplitude = 0.0f;                 // �g�̐U���i�傫���j
	float frequency = 1.0f;                 // ���g���i�g�̃X�s�[�h�j
	bool reverse = false;                   // �g�̌��������E���]���邩
	float timer = 0.0f;                     // �o�ߎ��ԁisin�g�Ɏg���j

public:
	EnemyBullet5();
	~EnemyBullet5();

public:
	// ����������
	virtual void Initialize() override;

	/// <summary>
	/// �X�V����
	/// </summary>
	/// <param name="delata_second">�P�t���[��������̎���</param>
	virtual void Update(float delta_second) override;

	/// <summary>
	/// �`�揈��
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

	// �I��������
	virtual void Finalize() override;

	/// <summary>
	/// �q�b�g�����Ƃ��̏���
	/// </summary>
	/// <param name="hit_object">�v���C���[���Ƃ�����A�����͓G�Ƃ��u���b�N�Ɠ����������ƂɂȂ�</param>
	void OnHitCollision(GameObjectBase* hit_object) override;

public:
	void SetWaveParameters(const Vector2D& base_velocity, float amplitude, float frequency, bool reverse);
};

