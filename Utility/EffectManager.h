#pragma once
#include "../Singleton.h"
#include "Vector2D.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>


enum EffectName
{
	eExprotion = 0,
	eExprotion2,
	eChenge,
	eExprotion3,
};

// EffectID を int型と同等にする
using EffectID = int;

class EffectManager : public Singleton<EffectManager>
{
	// 各エフェクトに項目設定
	struct Effect
	{
		EffectID id;                       // エフェクトを区別するID
		std::vector<int> image_handles;    // エフェクトの画像（）複数でも単体でも
		Vector2D position;                 // 描画位置
		float elapsed_time = 0.0f;         // 累積時間
		float frame_time = 0.1f;           // 各フレームを何秒表示するか
		int current_frame = 0;             // 現在表示しているフレーム番号
		bool loop = false;                 // エフェクトをループさせるか
		bool is_paused = false;            // 一時停止させるかどうか
		bool is_finished = false;          // 終了させるかどうか

		int alpha = 255;                   // 透明度
		float scale = 1.0f;  // 拡大率
		int z_layer = 0;                       // レイヤー

		// 初期化処理
		Effect(EffectID id_, const std::vector<int>& handles, const Vector2D& pos,
			float frame_time_sec, bool loop_flag)
			: id(id_), image_handles(handles), position(pos), frame_time(frame_time_sec), loop(loop_flag) {}
	};

public:
	EffectManager() = default;

	void LoadAllEffects();

	EffectID PlayerAnimation(EffectName effect_name, const Vector2D& position,
		float frame_time_sec, bool loop);

	// 更新処理
	void Update(const float& delta_second);

	// 描画処理
	void Draw();

	// 終了時処理
	void Finalize();

public:
	// 指定IDのアニメーションが終了しているかどうかを返す
	bool GetAnimationFinished(EffectID id) const;

	// 指定IDのアニメーションを即削除
	void RemoveAnimation(EffectID id);

	// α値の設定（０～２５５）
	void SetAlpha(EffectID id, int alpha);

	// 拡大率設定（1.0　＝　等倍）
	void SetScale(EffectID id, float scale);

	// 一時停止
	void PauseAnimation(EffectID id);

	// 再開
	void ResumeAnimation(EffectID id);

	// レイヤーの設定
	void SetZLayer(EffectID id, int z);

	// 位置情報の設定
	void SetPosition(EffectID id, const Vector2D& posi);

private:
	// 次に再生するID番号
	EffectID next_id = 0;

	// IDとアニメーションの対応マップ
	std::unordered_map<EffectID, std::unique_ptr<Effect>> animations;

	std::map<int, std::vector<int>> effect_images;
};

