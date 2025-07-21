#pragma once
#include "PlayerBulletBase.h"
class PlayerDefenceBullet :
    public PlayerBulletBase
{
private:
	bool is_collected = false;
public:
	PlayerDefenceBullet();
	~PlayerDefenceBullet();

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

