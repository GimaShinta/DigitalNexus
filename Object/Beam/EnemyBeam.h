#pragma once
#include "../GameObjectBase.h"
class EnemyBeam : public GameObjectBase
{
private:
	class Boss3* boss3;
	class Boss4* boss4;

	float beam_time = 0.0f;

	float alpha_timer = 0.0f;          // 点滅用のタイマー

	float shrink_duration = 1.0f;  // 縮小にかける時間
	float beam_lifetime = 8.0f;    // ビーム全体の生存時間（今と同じ）
	enum class BeamOwnerType
	{
		None,
		Boss2,
		Boss3,
	};

	BeamOwnerType beam_owner_type = BeamOwnerType::None;
	float min_thickness = 8.0f;
	float max_thickness = 48.0f;
	float min_thickness_d = 8.0f;
	float max_thickness_d = 48.0f;
	int beam_t_index = 10;
	int beam_b_index = 10;
	float growth_duration_b = 4.0f;
	float growth_duration_s = 0.5f;

	bool checked = true;

private:
	std::vector<int> beam_ts;
	std::vector<int> beam_bs;
	std::vector<int> beams_t;
	std::vector<int> beams_b;
	int beam_t = NULL;
	int beam_b = NULL;


	enum class BeamState
	{
		Warning,
		Firing,
		Holding,   // ← 追加
		Shrinking, // ← 追加！
	};

	BeamState state = BeamState::Warning;
	float warning_time = 1.5f;  // 予兆線の表示時間
	float state_timer = 0.0f;   // 状態の経過時間

public:
	EnemyBeam();
	~EnemyBeam();

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

	/// <summary>
	/// ヒット時処理
	/// </summary>
	/// <param name="hit_object">当たった相手</param>
	virtual void OnHitCollision(GameObjectBase* hit_object) override;

	void SetBoss3(class Boss3* p_boss);
	void SetBoss4(class Boss4* p_boss);

	template<typename T>
	T MyMax(const T& a, const T& b) {
		return (a > b) ? a : b;
	}
};

