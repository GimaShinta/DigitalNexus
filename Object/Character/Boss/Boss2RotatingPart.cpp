#include "Boss2RotatingPart.h"
#include "Boss2.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../GameObjectManager.h"
#include "../../../Utility/ResourceManager.h"
#include <cmath>

namespace
{
    constexpr float PI = 3.1415926535f;
    inline float DegToRad(float deg)
    {
        return deg * (PI / 180.0f);
    }
}

Boss2RotatingPart::Boss2RotatingPart()
{}

void Boss2RotatingPart::Initialize()
{
    z_layer = 3;
    box_size = Vector2D(20.0f, 20.0f);

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemyPart;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    auto imgs = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy74_a.png", 6, 6, 1, 48, 48);
    image = imgs.empty() ? -1 : imgs[0];

    shot_timer = 0.0f;
    mode_timer = 0.0f;
    spiral_phase = DegToRad(angle_offset); // �p�[�c���Ɉʑ������炷
    pattern = BulletPattern::Spiral;
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

    // ���݊p�x�i�I�t�Z�b�g���݁j
    const float rad = DegToRad(angle + angle_offset);

    // Boss�̎�������
    const Vector2D boss_pos = boss->GetLocation();
    location.x = boss_pos.x + std::cos(rad) * radius;
    location.y = boss_pos.y + std::sin(rad) * radius;

    // ---- �e���i�e�S���̂݁j ----
    if (is_bullet_part)
    {
        mode_timer += delta_second;
        shot_timer += delta_second;

        // �p�^�[���ؑ�
        if (mode_timer >= pattern_cycle)
        {
            mode_timer = 0.0f;
            shot_timer = 0.0f;

            switch (pattern)
            {
                case BulletPattern::Spiral: pattern = BulletPattern::NWay; break;
                case BulletPattern::NWay:   pattern = BulletPattern::Ring; break;
                case BulletPattern::Ring:   pattern = BulletPattern::Spiral; break;
            }
        }

        // �p�^�[�����Ƃ̒e����
        switch (pattern)
        {
            case BulletPattern::Spiral:
                if (shot_timer >= interval_spiral)
                {
                    shot_timer = 0.0f;
                    // �p�[�c�̊O�������x�[�X + �X�p�C������]
                    float a = rad + spiral_phase;
                    Vector2D dir(std::cos(a), std::sin(a));
                    FireBullet(dir, speed_spiral);
                    // ���X�Ɋp�x��i�߂�i�p�[�c���ɉ�]�������ς��悤�ɍH�v�j
                    float delta = DegToRad(12.0f) * ((std::fmod(angle_offset, 120.0f) < 60.0f) ? 1.0f : -1.0f);
                    spiral_phase += delta;
                }
                break;

            case BulletPattern::NWay:
                if (shot_timer >= interval_nway)
                {
                    shot_timer = 0.0f;
                    int m = nway_count;
                    if (m < 1) break;
                    if (m == 1)
                    {
                        FireBullet(Vector2D(std::cos(rad), std::sin(rad)), speed_nway);
                    }
                    else
                    {
                        // ���S��������ɍ��E�֐���
                        float spread = DegToRad(nway_spread_deg);
                        float start = -spread * 0.5f;
                        float step = (m > 1) ? spread / (m - 1) : 0.0f;
                        for (int i = 0; i < m; ++i)
                        {
                            float a = rad + start + step * i;
                            FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_nway);
                        }
                    }
                }
                break;

            case BulletPattern::Ring:
                if (shot_timer >= interval_ring)
                {
                    shot_timer = 0.0f;
                    int cnt = ring_count;
                    if (cnt < 3) cnt = 3;
                    float base = rad; // �p�[�c�̊O��������ɉ�
                    for (int i = 0; i < cnt; ++i)
                    {
                        float a = base + (2.0f * PI / cnt) * i;
                        FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_ring);
                    }
                }
                break;
        }
    }

    __super::Update(delta_second);
}

void Boss2RotatingPart::Draw(const Vector2D& screen_offset) const
{
    if (image == -1) return;
    DrawRotaGraph(location.x, location.y, 2.0f, 0.0f, image, TRUE);
}

float Boss2RotatingPart::GetAngleGlobal() const
{
    return angle + angle_offset;
}

void Boss2RotatingPart::FireBullet(const Vector2D& dir, float speed)
{
    // EnemyBullet1 �� Update ���� (velocity * 200) ���g���z��̂��߁A���x�� 200 ��ɐ��K�����Đݒ�
    const float kBase = 200.0f;
    Vector2D v = dir;
    // �O�̂��ߐ��K��
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len > 0.0001f)
    {
        v.x /= len; v.y /= len;
    }
    v = v * (speed / kBase);

    auto bullet = GameObjectManager::GetInstance()->CreateObject<EnemyBullet1>(location);
    bullet->SetVelocity(v);
}
