#pragma once
#include "../../GameObjectBase.h"
class EnemyBullet6 :
    public GameObjectBase
{
private:
	enum Phase { MoveIn, Stop, Homing, Straight };
	Phase phase = MoveIn;

	float timer = 0.0f;
	float stop_duration = 1.0f;
	float homing_speed = 200.0f;
	float initial_speed = 400.0f;
	Vector2D last_velocity;
	Vector2D homing_velocity;

	float total_move_duration = 0.8f; // �O�i�ɂ����鎞�ԁi�������ԁj
	float total_homing_duration = 0.6f; // �ǔ��ɂ������������
	float target_stop_y = 0.0f;
	float homing_end_distance = 200.0f; 
	class Player* player;
	Vector2D homing_direction;
	Vector2D target_stop_position;

public:
	EnemyBullet6();
	~EnemyBullet6();

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

	void SetPlayer(class Player* p);

	// ���`���
	float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}
	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}


	void SetPhaseParams(float init_speed, float stop_dur, float homing_spd, float stop_x_random_offset, float stop_y_random_offset);
};

