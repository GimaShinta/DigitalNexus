#pragma once
#include "PlayerBulletBase.h"
class PlayerDefenceBullet :
    public PlayerBulletBase
{
private:
	bool is_collected = false;
	float angle_deg = 0.0f; // ’e‚Ì‰ñ“]Šp“xiŒ©‚½–Ú—pj
public:
	PlayerDefenceBullet();
	~PlayerDefenceBullet();

public:
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;

	// I—¹ˆ—
	virtual void Finalize() override;

	void SetDirection(float degree);
	float GetAngle() const { return angle_deg; }
	/// <summary>
	/// •`‰æˆ—
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

public:
	/// <summary>
	/// ƒqƒbƒgˆ—
	/// </summary>
	/// <param name="hit_object">“–‚½‚Á‚½‘Šè</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
};

