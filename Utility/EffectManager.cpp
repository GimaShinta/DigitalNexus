#include "EffectManager.h"
#include "ResourceManager.h"
#include "DxLib.h"
#include <algorithm>

// �G�t�F�N�g�摜�̓ǂݍ���
void EffectManager::LoadAllEffects()
{
	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();

	effect_images[EffectName::eExprotion] = rm->GetImages("Resource/Image/Effect/E_Explosion.png", 54, 9, 6, 517, 517);
	effect_images[EffectName::eExprotion2] = rm->GetImages("Resource/Image/Effect/E_Explosion2.png", 54, 9, 6, 355, 365);

	std::vector<int> charge = rm->GetImages("Resource/Image/Effect/E_Charged.png", 80, 10, 8, 357, 295);
	std::vector<int> charge_2;
	// 25�����̃G�t�F�N�g�摜���擾
	for (int i = 0; i < 25; i++)
	{
		charge_2.push_back(charge[i]);
	}
	effect_images[EffectName::eChenge] = charge_2;
}

// �G�t�F�N�g�̍Đ�
EffectID EffectManager::PlayerAnimation(EffectName effect_name, const Vector2D& position, float frame_time_sec, bool loop)
{
	auto it = effect_images.find(effect_name);

	if (it == effect_images.end() || it->second.empty())
	{
		// �f�o�b�O���̒ʒm
#ifndef NDEBUG
		printfDx("Error: �G�t�F�N�g�����[�h����Ă��Ȃ� (%d)\n", effect_name);
#endif

		return -1;  // ������ID��Ԃ��ČĂяo�����Ŗ���������
	}

	const std::vector<int>& handles = it->second;

	EffectID id = next_id++;
	animations[id] = std::make_unique<Effect>(id, handles, position,
		frame_time_sec, loop);
	return id;
}

// �G�t�F�N�g�̍X�V����
void EffectManager::Update(const float& delta_second)
{
	for (auto it = animations.begin(); it != animations.end(); )
	{
		Effect* anim = it->second.get();

		// ��~�� or �ꎞ��~���̃A�j���[�V�����̓X�L�b�v
		if (anim->is_paused)
		{
			++it;
			continue;
		}

		// ���Ԍo�߂Ńt���[���i�s
		anim->elapsed_time += delta_second;
		if (anim->elapsed_time >= anim->frame_time)
		{
			anim->elapsed_time -= anim->frame_time;
			anim->current_frame++;

			if (anim->current_frame >= static_cast<int>(anim->image_handles.size()))
			{
				if (anim->loop)
				{
					anim->current_frame = 0;  // ���[�v�Đ�
				}
				else
				{
					anim->current_frame = static_cast<int>(anim->image_handles.size()) - 1;
					anim->is_finished = true;  // �Đ�����
				}
			}
		}

		// �Đ������Ȃ�폜
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

// �G�t�F�N�g�̕`�揈��
void EffectManager::Draw()
{
	std::vector<Effect*> draw_list;

	// �ꎞ��~���Ă��Ȃ��A�j���[�V����������`��Ώۂɂ���
	for (const auto& pair : animations)
	{
		if (!pair.second->is_paused)
		{
			draw_list.push_back(pair.second.get());
		}
	}

	// ���C���[���Ƀ\�[�g�i�l���傫���قǑO�ʁj
	std::sort(draw_list.begin(), draw_list.end(), [](Effect* a, Effect* b)
		{
			return a->z_layer < b->z_layer;
		});

	// �`�揈��
	for (auto* anim : draw_list)
	{
		int handle = anim->image_handles[anim->current_frame];

		// �����x��K�p
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, anim->alpha);

		// �g�嗦�Ɖ�]�p�i�����ł� 0�j���w�肵�ĕ`��
		DrawRotaGraphF(anim->position.x, anim->position.y, anim->scale, 0.0f, handle, true);

		// �u�����h���[�h�����Z�b�g
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// �I�����ɃG�t�F�N�g�̍폜
void EffectManager::Finalize()
{
	animations.clear();
}

// �w�肵���G�t�F�N�g���Đ�����Ă��邩������
bool EffectManager::GetAnimationFinished(EffectID id) const
{
	auto it = animations.find(id);
	if (it != animations.end())
	{
		return it->second->is_finished;
	}
	return false;
}

// �w�肵���G�t�F�N�g�̑��폜
void EffectManager::RemoveAnimation(EffectID id)
{
	animations.erase(id);
}

// �w�肵���G�t�F�N�g�̓����x��ݒ肷��
void EffectManager::SetAlpha(EffectID id, int alpha)
{
	if (animations.count(id))
	{
		if (alpha < 0) alpha = 0;
		else if (alpha > 255) alpha = 255;

		animations[id]->alpha = alpha;
	}
}

// �w�肵���G�t�F�N�g�̑傫����ݒ肷��
void EffectManager::SetScale(EffectID id, float scale)
{
	if (animations.count(id))
	{
		animations[id]->scale = scale;
	}
}

// �w�肵���G�t�F�N�g���~������
void EffectManager::PauseAnimation(EffectID id)
{
	if (animations.count(id))
	{
		animations[id]->is_paused = true;
	}
}

// �w�肵���G�t�F�N�g���čĐ�������
void EffectManager::ResumeAnimation(EffectID id)
{
	if (animations.count(id))
	{
		animations[id]->is_paused = false;
	}
}

// �w�肵���G�t�F�N�g�̃��C���[��ύX����
void EffectManager::SetZLayer(EffectID id, int z)
{
	if (animations.count(id))
	{
		animations[id]->z_layer = z; 
	}
}

// �w�肵���G�t�F�N�g�̍Đ��ʒu��ύX����
void EffectManager::SetPosition(EffectID id, const Vector2D& posi)
{
	if (animations.count(id))
	{
		animations[id]->position = posi;
	}
}
