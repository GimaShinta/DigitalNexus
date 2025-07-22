#pragma once
#include "PlayerBulletBase.h"
class PlayerDefenceBullet :
    public PlayerBulletBase
{
private:
	bool is_collected = false;
	float angle_deg = 0.0f; // �e�̉�]�p�x�i�����ڗp�j
public:
	PlayerDefenceBullet();
	~PlayerDefenceBullet();

public:
	// ����������
	virtual void Initialize() override;

	// �I��������
	virtual void Finalize() override;

	void SetDirection(float degree);
	float GetAngle() const { return angle_deg; }
	/// <summary>
	/// �`�揈��
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

public:
	/// <summary>
	/// �q�b�g������
	/// </summary>
	/// <param name="hit_object">������������</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
};

