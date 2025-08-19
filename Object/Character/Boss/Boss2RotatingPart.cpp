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
    spiral_phase = DegToRad(angle_offset); // パーツ毎に位相をずらす
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

    // 現在角度（オフセット込み）
    const float rad = DegToRad(angle + angle_offset);

    // Bossの周りを回る
    const Vector2D boss_pos = boss->GetLocation();
    location.x = boss_pos.x + std::cos(rad) * radius;
    location.y = boss_pos.y + std::sin(rad) * radius;

    // ---- 弾幕（弾担当のみ） ----
    if (is_bullet_part)
    {
        mode_timer += delta_second;
        shot_timer += delta_second;

        // パターン切替
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

        // パターンごとの弾生成
        switch (pattern)
        {
            case BulletPattern::Spiral:
                if (shot_timer >= interval_spiral)
                {
                    shot_timer = 0.0f;
                    // パーツの外向きをベース + スパイラル回転
                    float a = rad + spiral_phase;
                    Vector2D dir(std::cos(a), std::sin(a));
                    FireBullet(dir, speed_spiral);
                    // 徐々に角度を進める（パーツ毎に回転方向が変わるように工夫）
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
                        // 中心方向を基準に左右へ扇状に
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
                    float base = rad; // パーツの外向きを基準に回す
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
    // EnemyBullet1 は Update 内で (velocity * 200) を使う想定のため、速度を 200 基準に正規化して設定
    const float kBase = 200.0f;
    Vector2D v = dir;
    // 念のため正規化
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len > 0.0001f)
    {
        v.x /= len; v.y /= len;
    }
    v = v * (speed / kBase);

    auto bullet = GameObjectManager::GetInstance()->CreateObject<EnemyBullet1>(location);
    bullet->SetVelocity(v);
}
