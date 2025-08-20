#pragma once
#include "EnemyBulletBase.h"
class EnemyBullet3 :
    public EnemyBulletBase
{
private:
	int attack_pattrn = 0;
	int input_count = 0;
	float move_time = 0.0f;

	std::vector<int> bullet;
	std::vector<int> bullet_1;
	std::vector<int> bullet_2;
	std::vector<int> bullet_3;

	std::vector<std::vector<int>> bullets;

public:
	EnemyBullet3();
	~EnemyBullet3();

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

	void SetAttackPattrn(int pattrn);
};

