#pragma once
#include "PlayerBulletBase.h"
class PlayerAttackBullet :
    public PlayerBulletBase
{
public:
	PlayerAttackBullet();
	~PlayerAttackBullet();

public:
	// ����������
	virtual void Initialize() override;

	// �I��������
	virtual void Finalize() override;

public:
	/// <summary>
	/// �q�b�g������
	/// </summary>
	/// <param name="hit_object">������������</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
};

