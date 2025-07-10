#pragma once
#include "../GameObjectBase.h"
class EnemyBullet4 :
	public GameObjectBase
{
private:
	Vector2D suck_center;
	float move_time = 0.0f;

public:
	bool is_wave_reflected = false; // ”g‚ÌŒü‚«‚ª”½“]‚µ‚Ä‚¢‚é‚©

private:
	enum class ShotState
	{
		SpreadOut,
		PullIn,
		Explode
	};
	ShotState shot_state = ShotState::SpreadOut;

private:
	Vector2D base_velocity;     // ‘Oi•ûŒü‚Ì‘¬“x
	float wave_amplitude;       // ”g‚ÌU‚ê•
	float wave_frequency;       // ”g‚Ìü”g”

public:
	EnemyBullet4();
	~EnemyBullet4();

public:
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;

	/// <summary>
	/// XVˆ—
	/// </summary>
	/// <param name="delata_second">‚PƒtƒŒ[ƒ€“–‚½‚è‚ÌŠÔ</param>
	virtual void Update(float delta_second) override;

	/// <summary>
	/// •`‰æˆ—
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

	// I—¹ˆ—
	virtual void Finalize() override;

public:
	void SetSuckCenter(const Vector2D& center);
	void SetWaveParameters(float amplitude, float frequency);
	void SetWaveReflected(bool reflected); // İ’èŠÖ”
};

