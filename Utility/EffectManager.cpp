#include "EffectManager.h"
#include "ResourceManager.h"
#include "DxLib.h"
#include <algorithm>

void EffectManager::LoadAllEffects()
{
	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();

	effect_images[EffectName::eExprotion] = rm->GetImages("Resource/Image/Effect/E_Explosion.png", 54, 9, 6, 517, 517);
	effect_images[EffectName::eExprotion2] = rm->GetImages("Resource/Image/Effect/E_Explosion2.png", 54, 9, 6, 355, 365);

	std::vector<int> charge = rm->GetImages("Resource/Image/Effect/E_Charged.png", 80, 10, 8, 357, 295);
	std::vector<int> charge_2;

	for (int i = 0; i < 25; i++)
	{
		charge_2.push_back(charge[i]);
	}

	effect_images[EffectName::eChenge] = charge_2;
}

EffectID EffectManager::PlayerAnimation(EffectName effect_name, const Vector2D& position, float frame_time_sec, bool loop)
{
	auto it = effect_images.find(effect_name);

	// ★防御コード：未登録 or 空 vector を検出
	if (it == effect_images.end() || it->second.empty())
	{
		// デバッグ時の通知
#ifndef NDEBUG
		printfDx("Error: エフェクトがロードされていない (%d)\n", effect_name);
#endif

		return -1;  // 無効なIDを返して呼び出し元で無視させる
	}

	const std::vector<int>& handles = it->second;

	EffectID id = next_id++;
	animations[id] = std::make_unique<Effect>(id, handles, position,
		frame_time_sec, loop);
	return id;
}

void EffectManager::Update(const float& delta_second)
{
	for (auto it = animations.begin(); it != animations.end(); )
	{
		Effect* anim = it->second.get();

		// 停止中 or 一時停止中のアニメーションはスキップ
		if (anim->is_paused)
		{
			++it;
			continue;
		}

		// 時間経過でフレーム進行
		anim->elapsed_time += delta_second;
		if (anim->elapsed_time >= anim->frame_time)
		{
			anim->elapsed_time -= anim->frame_time;
			anim->current_frame++;

			if (anim->current_frame >= static_cast<int>(anim->image_handles.size()))
			{
				if (anim->loop)
				{
					anim->current_frame = 0;  // ループ再生
				}
				else
				{
					anim->current_frame = static_cast<int>(anim->image_handles.size()) - 1;
					anim->is_finished = true;  // 再生完了
				}
			}
		}

		// 再生完了なら削除
		if (anim->is_finished)
		{
			it = animations.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void EffectManager::Draw()
{
	std::vector<Effect*> draw_list;

	// 一時停止していないアニメーションだけを描画対象にする
	for (const auto& pair : animations)
	{
		if (!pair.second->is_paused)
		{
			draw_list.push_back(pair.second.get());
		}
	}

	// レイヤー順にソート（値が大きいほど前面）
	std::sort(draw_list.begin(), draw_list.end(), [](Effect* a, Effect* b)
		{
			return a->z_layer < b->z_layer;
		});

	// 描画処理
	for (auto* anim : draw_list)
	{
		int handle = anim->image_handles[anim->current_frame];

		// 透明度を適用
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, anim->alpha);

		// 拡大率と回転角（ここでは 0）を指定して描画
		DrawRotaGraphF(anim->position.x, anim->position.y, anim->scale, 0.0f, handle, true);

		// ブレンドモードをリセット
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void EffectManager::Finalize()
{
	animations.clear();
}

bool EffectManager::GetAnimationFinished(EffectID id) const
{
	auto it = animations.find(id);
	if (it != animations.end())
	{
		return it->second->is_finished;
	}
	return false;
}

void EffectManager::RemoveAnimation(EffectID id)
{
	animations.erase(id);
}

void EffectManager::SetAlpha(EffectID id, int alpha)
{
	if (animations.count(id))
	{
		if (alpha < 0) alpha = 0;
		else if (alpha > 255) alpha = 255;

		animations[id]->alpha = alpha;
	}
}

void EffectManager::SetScale(EffectID id, float scale)
{
	if (animations.count(id))
	{
		animations[id]->scale = scale;
	}
}

void EffectManager::PauseAnimation(EffectID id)
{
	if (animations.count(id))
	{
		animations[id]->is_paused = true;
	}
}

void EffectManager::ResumeAnimation(EffectID id)
{
	if (animations.count(id))
	{
		animations[id]->is_paused = false;
	}
}

void EffectManager::SetZLayer(EffectID id, int z)
{
	if (animations.count(id))
	{
		animations[id]->z_layer = z;  // ← 修正済み：代入漏れを修正
	}
}

void EffectManager::SetPosition(EffectID id, const Vector2D& posi)
{
	if (animations.count(id))
	{
		animations[id]->position = posi;
	}
}
