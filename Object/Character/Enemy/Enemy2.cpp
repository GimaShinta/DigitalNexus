#include "Enemy2.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/ResourceManager.h"

Enemy2::Enemy2() {}
Enemy2::~Enemy2() {}

void Enemy2::Initialize()
{
    EnemyBase::Initialize();
    hp = 20;
    z_layer = 2;
    box_size = 16;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_a.png", 4, 4, 1, 32, 32);
    image = images[0];

    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    state = Enemy2State::Appearing;
    move_mode = Enemy2MoveMode::Normal;

    appear_timer = 0.0f;
    float_timer = 0.0f;
    alpha = 0;
    scale = scale_min;
}

void Enemy2::SetAppearParams(const Vector2D& start, const Vector2D& end, float time)
{
    start_location = start;
    target_location = end;
    appear_duration = time;
    location = start_location;
    state = Enemy2State::Appearing;
    appear_timer = 0.0f;
    float_timer = 0.0f;
    alpha = 0;
    scale = scale_min;
}

void Enemy2::SetChaseAndFeint(bool from_left, Player* p)
{
    move_mode = Enemy2MoveMode::ChaseFeint;
    feint_from_left = from_left;
    player_ref = p;
    feint_triggered = false;
}

void Enemy2::SetDoubleHelixMove(bool from_left, int index)
{
    move_mode = Enemy2MoveMode::DoubleHelix;
    helix_from_left = from_left;
    helix_order = index;
    helix_angle = from_left ? 180.0f : 0.0f;
}

void Enemy2::Update(float delta_second)
{
    appear_timer += delta_second;

    switch (state)
    {
    case Enemy2State::Appearing:
    {
        // 出現演出
        float t = appear_timer / appear_duration;
        if (t > 1.0f) t = 1.0f;
        float ease_t = t * t * (3 - 2 * t); // smoothstep

        Vector2D prev = location;
        location = start_location + (target_location - start_location) * ease_t;
        velocity = (location - prev) / delta_second;

        scale = scale_min + (scale_max - scale_min) * ease_t;
        alpha = static_cast<int>(255 * ease_t);

        if (t >= 1.0f)
        {
            state = Enemy2State::Floating;
            base_location = location;
            float_timer = 0.0f;
            collision.is_blocking = true;
            collision.object_type = eObjectType::eEnemy;
        }
        break;
    }

    case Enemy2State::Floating:
    {
        float_timer += delta_second;

        switch (move_mode)
        {
        case Enemy2MoveMode::ChaseFeint:
        {
            if (player_ref)
            {
                Vector2D dir = player_ref->GetLocation() - location;
                dir.Normalize();

                if (float_timer < 2.0f)
                {
                    // プレイヤー追尾
                    location += dir * delta_second * 220.0f;
                }
                else
                {
                    // フェイントで左右逃げ
                    if (!feint_triggered)
                    {
                        feint_triggered = true;
                    }
                    float side = feint_from_left ? -1.0f : 1.0f;
                    location.x += side * delta_second * 300.0f;
                    location.y -= delta_second * 100.0f;
                }
            }
            break;
        }

        case Enemy2MoveMode::DoubleHelix:
        {
            // DNAのように交差
            helix_angle += 4.0f;
            float rad = helix_angle * DX_PI_F / 180.0f;

            float advance = (float_timer + helix_order * 0.2f) * 120.0f;
            float x_offset = cosf(rad) * helix_radius;
            float y_offset = sinf(rad) * 40.0f;

            if (helix_from_left)
                location = Vector2D(start_location.x + advance, target_location.y + y_offset);
            else
                location = Vector2D(start_location.x - advance, target_location.y + y_offset);
            break;
        }

        default:
        {
            // 通常漂い
            location.x = base_location.x + sinf(float_timer * 1.2f) * 25.0f;
            location.y = base_location.y + sinf(float_timer * 2.0f) * 10.0f;
            break;
        }
        }

        // 攻撃
        if (fmodf(float_timer, 2.0f) < 0.02f)
        {
            Shot(0.004f);
        }

        if (float_timer >= 4.5f)
        {
            state = Enemy2State::Leaving;
        }
        break;
    }

    case Enemy2State::Leaving:
    {
        location.y -= delta_second * 220.0f;
        if (location.y + box_size.y < 0)
        {
            is_destroy = true;
        }
        break;
    }
    }

    // 撃破判定
    if (hp <= 0)
    {
        is_destroy = true;
        DropItems();
        SEManager::GetInstance()->PlaySE(SE_NAME::Destroy);
        EffectManager::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        Singleton<ScoreData>::GetInstance()->AddScore(1500);
    }

    // アニメーション更新
    animation_time += delta_second;
    if (animation_time >= 0.1f)
    {
        animation_time = 0.0f;
        animation_count = (animation_count + 1) % images.size();
        image = images[animation_count];
    }

    EnemyBase::Update(delta_second);
}

void Enemy2::Draw(const Vector2D& screen_offset) const
{
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy2::Finalize() {}

void Enemy2::Shot(float speed)
{
    if (!player) return;
    Vector2D dir = player->GetLocation() - location;
    dir.Normalize();

    auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
    if (shot) shot->SetVelocity(dir * speed);
}
