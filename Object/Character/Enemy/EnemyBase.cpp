#include "EnemyBase.h"
#include"../../../Utility/ScoreData.h"
#include "../../Item/Exp/Exp.h"
#include "../../Item/PowerUp/PowerUp.h"
#include "../../Item/Shield/Shield.h"
#include "../../Item/MaxCharge/MaxCharge.h"

EnemyBase::EnemyBase() : hp(0.0f), on_hit(false)
{
}

EnemyBase::~EnemyBase()
{
}

// ����������
void EnemyBase::Initialize()
{
}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="delata_second">1�t���[��������̎���</param>
void EnemyBase::Update(float delta_second)
{
	delta = delta_second;

	// �e�N���X�̍X�V�������Ăяo��
	__super::Update(delta_second);
}

/// <summary>
/// �`�揈��
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBase::Draw(const Vector2D& screen_offset) const
{
	// �e�N���X�̕`�揈�����Ăяo��
	__super::Draw(screen_offset);
}

// �I��������
void EnemyBase::Finalize()
{
}

/// <summary>
/// �q�b�g������
/// </summary>
/// <param name="hit_object">������������</param>
void EnemyBase::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::eAttackShot)
	{
		if (on_hit == false)
		{
			hp -= 10;
			on_hit = true;
		}
		else
		{
			on_hit = false;
		}
	}
	if (hit_object->GetCollision().object_type == eObjectType::eBeam)
	{
		beam_damage_timer += delta;

		if (beam_damage_timer >= 0.05f)
		{
			hp -= 10;
			beam_damage_timer = 0;
		}
	}
}

void EnemyBase::SetPlayer(Player* p)
{
	player = p;
}

void EnemyBase::DropItems()
{
	GameObjectManager* manager = Singleton<GameObjectManager>::GetInstance();

	// Exp�i�m��j
	Exp* exp = manager->CreateObject<Exp>(location);
	exp->SetPlayer(player);

	// ��̗����Ŕ��f�i�ő��1��ނ����o�Ȃ��j
	int dropRoll = rand() % 100;

	if (dropRoll < 1) {
		MaxCharge* charge = manager->CreateObject<MaxCharge>(location);
		charge->SetPlayer(player);
	}
	else if (dropRoll < 6) {
		Shield* shi = manager->CreateObject<Shield>(location);
		shi->SetPlayer(player);
	}
}
