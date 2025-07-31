#include "Enemy1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/ResourceManager.h"

Enemy1::Enemy1() {}                  // �R���X�g���N�^
Enemy1::~Enemy1() {}                 // �f�X�g���N�^ �� ���ꂪ�K�v�I�I


void Enemy1::Initialize()
{
    EnemyBase::Initialize();
    hp = 8;
    z_layer = 2;
    box_size = 12;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_a.png", 4, 4, 1, 32, 32);
    image = images[0];

    collision.is_blocking = false;  // �o�����͖��G
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    scale = 2.0f;
    appear_timer = 0.0f;
    float_timer = 0.0f;
    state = Enemy1State::Appearing;

    alpha = 0;
}

void Enemy1::SetAppearParams(const Vector2D& start, const Vector2D& end, float time) {
    start_location = start;
    target_location = end;
    appear_duration = time;
    location = start_location;
    state = Enemy1State::Appearing;
    appear_timer = 0.0f;
    float_timer = 0.0f;
    alpha = 0;
    scale = scale_min;
}

//void Enemy1::Update(float delta_second)
//{
//    appear_timer += delta_second;
//
//    switch (state)
//    {
//    case Enemy1State::Appearing:
//    {
//        float t = appear_timer / appear_duration;
//        if (t > 1.0f) t = 1.0f;
//
//        float ease = t * t * (3 - 2 * t);  // smoothstep
//
//        Vector2D prev = location;
//        location = start_location + (target_location - start_location) * ease;
//        velocity = (location - prev) / delta_second;
//
//        scale = 0.3f + (1.5f * ease);
//        alpha = static_cast<int>(255 * ease);
//
//        if (t >= 1.0f)
//        {
//            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
//            Shot(0.003f);
//            state = Enemy1State::Floating;
//            base_location = location;
//            float_timer = 0.0f;
//        }
//        break;
//    }
//    case Enemy1State::Floating:
//    {
//        float_timer += delta_second;
//        location.x = base_location.x + sinf(float_timer * 1.5f) * 10.0f;
//        location.y = base_location.y + sinf(float_timer * 2.0f) * 5.0f;
//
//        if (float_timer >= 3.0f)
//        {
//            state = Enemy1State::Leaving;
//        }
//        break;
//    }
//    case Enemy1State::Leaving:
//    {
//        location.y -= delta_second * 300.0f;
//        if (location.y + box_size.y < 0)
//        {
//            is_destroy = true;
//        }
//        break;
//    }
//    }
//
//    // �o����͓����蔻���L���ɂ���
//    if (state == Enemy1State::Floating || state == Enemy1State::Leaving)
//    {
//        collision.is_blocking = true;
//        collision.object_type = eObjectType::eEnemy;
//        collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };
//    }
//
//    if (hp <= 0)
//    {
//        is_destroy = true;
//        DropItems();
//        SEManager::GetInstance()->PlaySE(SE_NAME::Destroy);
//        EffectManager::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
//
//        Singleton<ScoreData>::GetInstance()->AddScore(500);
//    }
//
//    animation_time += delta_second;
//    if (animation_time >= 0.1f)
//    {
//        animation_time = 0.0f;
//        animation_count = (animation_count + 1) % images.size();
//        image = images[animation_count];
//    }
//
//    EnemyBase::Update(delta_second);
//}
void Enemy1::Update(float delta_second) {
    appear_timer += delta_second;

    switch (state) {
    case Enemy1State::Appearing: {
        float t = appear_timer / appear_duration;
        if (t > 1.0f) t = 1.0f;

        float ease_t = t * t * (3 - 2 * t);

        Vector2D prev = location;
        location = start_location + (target_location - start_location) * ease_t;
        velocity = (location - prev) / delta_second;

        scale = scale_min + (scale_max - scale_min) * ease_t;
        alpha = static_cast<int>(255 * ease_t);

        if (t >= 1.0f) {
            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
            Shot(0.003f);
            state = Enemy1State::Floating;
            base_location = location;
            float_timer = 0.0f;
        }
        break;
    }

    case Enemy1State::Floating: {
        float_timer += delta_second;
        location.x = base_location.x + sinf(float_timer * 1.5f) * 10.0f;
        location.y = base_location.y + sinf(float_timer * 2.0f) * 5.0f;

        if (float_timer >= 3.0f) {
            state = Enemy1State::Leaving;
        }
        break;
    }
    case Enemy1State::Leaving: {
        location.y -= delta_second * 300.0f;
        if (location.y + box_size.y < 0) {
            is_destroy = true;
        }
        break;
    }
    }

    // �����蔻���ON/OFF
    collision.is_blocking = (state != Enemy1State::Appearing);
    collision.object_type = collision.is_blocking ? eObjectType::eEnemy : eObjectType::eNone;

    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        SEManager::GetInstance()->PlaySE(SE_NAME::Destroy);
        EffectManager::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
    }

    // �A�j���[�V����
    animation_time += delta_second;
    if (animation_time >= 0.1f) {
        animation_time = 0.0f;
        animation_count = (animation_count + 1) % images.size();
        image = images[animation_count];
    }

    EnemyBase::Update(delta_second);
}

void Enemy1::Draw(const Vector2D& screen_offset) const
{
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy1::Finalize() {}

void Enemy1::Shot(float speed)
{
    if (!player) return;
    Vector2D dir = player->GetLocation() - location;
    dir.Normalize();
    auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
    if (shot) shot->SetVelocity(dir * speed);
}

//void Enemy1::SetPattern(Enemy1Pattern new_pattern)
//{
//    pattern = new_pattern;
//    pattern_timer = 0.0f;
//}
