#include "EnemyBullet5.h"

EnemyBullet5::EnemyBullet5()
{
}

EnemyBullet5::~EnemyBullet5()
{
}

void EnemyBullet5::Initialize()
{
	z_layer = 4;
	box_size = 12;

	// �����蔻��̃I�u�W�F�N�g�ݒ�
	collision.is_blocking = true;
	// �����̃I�u�W�F�N�g�^�C�v
	collision.object_type = eObjectType::eEnemyShot;
	// �����鑊��̃I�u�W�F�N�g�^�C�v
	collision.hit_object_type.push_back(eObjectType::ePlayer);
	collision.hit_object_type.push_back(eObjectType::eDefenceShot);

	// �������ǂ����itrue�Ȃ瓮���Afalse�Ȃ�~�܂�j
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime81.png", 6, 6, 1, 24, 24)[0];

}

void EnemyBullet5::Update(float delta_second)
{
	// �e����ʊO�ɏo����폜�i�ړ������ɂ���Ĕ����ς���j
	if (location.y < -50.0f || // �����
		location.y > D_WIN_MAX_Y + 50.0f ||
		location.x > 990.0f ||
		location.x < 240.0f) // ������
	{
		is_destroy = true;
	}


	timer += delta_second;

	float offset = sinf(timer * frequency) * amplitude;
	if (reverse) offset *= -1;

	// ���ɔg�ł悤�ɐi�s
	Vector2D velocity = base_velocity + Vector2D(offset, 0.0f);

	location += velocity * delta_second;
}

void EnemyBullet5::Draw(const Vector2D& screen_offset) const
{
	DrawRotaGraph(location.x, location.y, 1.9f, 0.0f, image, TRUE);
}

void EnemyBullet5::Finalize()
{
}

void EnemyBullet5::OnHitCollision(GameObjectBase* hit_object)
{
}

void EnemyBullet5::SetWaveParameters(const Vector2D& base_velocity, float amplitude, float frequency, bool reverse)
{
	this->base_velocity = base_velocity;
	this->amplitude = amplitude;
	this->frequency = frequency;
	this->reverse = reverse;
	this->timer = 0.0f;
}
