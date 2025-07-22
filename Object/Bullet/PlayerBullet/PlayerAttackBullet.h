#pragma once
#include "PlayerBulletBase.h"
class PlayerAttackBullet :
    public PlayerBulletBase
{
public:
	PlayerAttackBullet();
	~PlayerAttackBullet();

public:
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;

	// I—¹ˆ—
	virtual void Finalize() override;

public:
	/// <summary>
	/// ƒqƒbƒgˆ—
	/// </summary>
	/// <param name="hit_object">“–‚½‚Á‚½‘Šè</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
};

