#include "Boss2RotatingPart.h"
#include "Boss2.h"
#include "../../../Utility/ResourceManager.h"
#include <cmath>

Boss2RotatingPart::Boss2RotatingPart() {}

void Boss2RotatingPart::Initialize()
{
    z_layer = 3;
    box_size = 20;

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemyPart;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    image = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy74_a.png", 6, 6, 1, 40, 40)[0];
}

void Boss2RotatingPart::SetUp(Boss2* boss, float radius, float angle_offset)
{
    this->boss = boss;
    this->radius = radius;
    this->angle_offset = angle_offset;
}

void Boss2RotatingPart::Update(float delta_second)
{
    if (!boss) return;

    angle += angle_speed * delta_second;
    if (angle >= 360.0f) angle -= 360.0f;

    float rad = (angle + angle_offset) * (3.1415926f / 180.0f);

    // Boss2‚ÌŽü‚è‚ð‰ñ‚é
    Vector2D boss_pos = boss->GetLocation();
    location.x = boss_pos.x + cosf(rad) * radius;
    location.y = boss_pos.y + sinf(rad) * radius;

    __super::Update(delta_second);
}

void Boss2RotatingPart::Draw(const Vector2D& screen_offset) const
{
    if (image == -1) return;
    DrawRotaGraph(location.x, location.y, 2.0f, 0.0f, image, TRUE);
}
