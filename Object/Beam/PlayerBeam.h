#pragma once
#include "../GameObjectBase.h"
class PlayerBeam :
    public GameObjectBase
{
private:
	enum class BeamState { Idle, Charging, Firing };
	BeamState beam_state = BeamState::Idle;

	float charge_time = 0.0f;
	const float charge_duration = 0.0f; // 1秒でチャージ完了
	bool charge_on = false;
private:
	bool Beam_flip = false;
	class Player* player;
	float destroy_time = 0.0f;
	float anim_time = 0.0f;
	bool anim_on = false;
private:
	float last_explosion_time = 0.0f; // 直近の爆発生成時間

private:
	std::vector<int> beam_ts;
	std::vector<int> beam_bs;
	int beam_t = NULL;
	int beam_b = NULL;
	int se;

public:
	PlayerBeam();
	~PlayerBeam();

public:
	// 初期化処理
	virtual void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delata_second">１フレーム当たりの時間</param>
	virtual void Update(float delta_second) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="screen_offset"></param>
	virtual void Draw(const Vector2D& screen_offset)const override;

	// 終了時処理
	virtual void Finalize() override;

public:
	/// <summary>
	/// ヒット時処理
	/// </summary>
	/// <param name="hit_object">当たった相手</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;
	void SetPlayer(class Player* p);
};

