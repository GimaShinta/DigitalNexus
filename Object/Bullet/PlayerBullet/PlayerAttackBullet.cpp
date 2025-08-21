#include "PlayerAttackBullet.h"

PlayerAttackBullet::PlayerAttackBullet()
{
}

PlayerAttackBullet::~PlayerAttackBullet()
{
}

void PlayerAttackBullet::Initialize()
{
	z_layer = 3;
	velocity.y = -2500.0f;
	box_size = Vector2D(5.0f, 14.0f);

	// �����蔻��̃I�u�W�F�N�g�ݒ�
	collision.is_blocking = true;
	// �����̃I�u�W�F�N�g�^�C�v
	collision.object_type = eObjectType::eAttackShot;
	// �����鑊��̃I�u�W�F�N�g�^�C�v
	collision.hit_object_type.push_back(eObjectType::eEnemy);

	// �������ǂ����itrue�Ȃ瓮���Afalse�Ȃ�~�܂�j
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Player/Shot/anime_effect16.png", 6, 6, 1, 8, 88)[2];

}

void PlayerAttackBullet::Finalize()
{
}

void PlayerAttackBullet::OnHitCollision(GameObjectBase* hit_object)
{
	EffectManager* am = Singleton<EffectManager>::GetInstance();
	SEManager* sm = Singleton<SEManager>::GetInstance();
	int anim_id = 0;
	// �����������肪�e��������
	if (hit_object->GetCollision().object_type == eObjectType::eEnemy)
	{
		float random_x = static_cast<float>(GetRand(20));
		if (GetRand(2) == 1)
		{
			sm->PlaySE(SE_NAME::Hit);
			sm->ChangeSEVolume(SE_NAME::Hit, 50);
			//PlaySoundMem(hit_se, DX_PLAYTYPE_BACK);
			random_x *= -1;
		}
		float random_y = static_cast<float>(GetRand(15));

		anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x - random_x, location.y - random_y), 0.01f, false);
		// �A�j���[�V�����̒ǉ��ݒ�
		am->SetAlpha(anim_id, 255);       // ������
		am->SetScale(anim_id, 0.2f);      // 1.5�{�g��

		is_destroy = true;
	}
	if (hit_object->GetCollision().object_type == eObjectType::eBoss)
	{
		sm->PlaySE(SE_NAME::Hit);
		sm->ChangeSEVolume(SE_NAME::Hit, 50);
		float random_x = static_cast<float>(GetRand(100));

		anim_id = am->PlayerAnimation(EffectName::eExprotion2, Vector2D(location.x, location.y - random_x), 0.01f, false);
		// �A�j���[�V�����̒ǉ��ݒ�
		am->SetAlpha(anim_id, 255);       // ������
		am->SetScale(anim_id, 0.2f);      // 1.5�{�g��

		is_destroy = true;
	}
}
