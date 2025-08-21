#include "Enemy1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/ResourceManager.h"

Enemy1::Enemy1() {}                  // コンストラクタ
Enemy1::~Enemy1() {}                 // デストラクタ ← これが必要！！
Enemy1::Enemy1(const Vector2D& pos)
{
    // 基本的な初期化
    location = pos;   // 初期位置を指定
}

void Enemy1::Initialize()
{
    EnemyBase::Initialize();
    hp = 8;
    z_layer = 2;
    box_size = 12;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako2/anime_enemy45.png", 6, 6, 1, 64, 64);
    image = images[0];

    collision.is_blocking = false;  // 出現中は無敵
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
    leave_delay = 2.5f + (GetRand(20) * 0.1f); // 2.5?4.5秒のランダム遅延

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
//    // 出現後は当たり判定を有効にする
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
            Shot(150.0f); // ←ここ、speedは 150.0f にすると視認しやすいです
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

        // ★降り始めを個体ごとにランダム遅延（2.5?4.5秒後）
        if (float_timer >= leave_delay)
        {
            state = Enemy1State::Leaving;
        }
        break;
    }

    case Enemy1State::Leaving: {
        // 左右にランダム方向の横移動を追加
        static float horizontal_speed = 100.0f;

        // 登場時に方向を決定しておくのがベストだが簡易的に左右ランダム
        if (velocity.x == 0) {
            velocity.x = (GetRand(1) == 0 ? -1 : 1) * horizontal_speed;
            velocity.y = 300.0f; // 下方向の速度
        }

        // 移動更新
        location.x += velocity.x * delta_second;
        location.y += velocity.y * delta_second;

        // 画面外に出たら削除
        if (location.y - box_size.y > D_WIN_MAX_Y ||
            location.x + box_size.x < 0 || location.x - box_size.x > D_WIN_MAX_X) {
            is_destroy = true;
        }
        break;
    }


    
}

    // 当たり判定のON/OFF
    collision.is_blocking = (state != Enemy1State::Appearing);
    collision.object_type = collision.is_blocking ? eObjectType::eEnemy : eObjectType::eNone;

    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        EffectManager* manager = Singleton<EffectManager>::GetInstance();
        SEManager::GetInstance()->PlaySE(SE_NAME::Destroy);
        SEManager::GetInstance()->PlaySE(SE_NAME::Dead1);
        Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
        EffectManager::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        manager->SetScale(anim_id, 0.5f);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
    }

    // アニメーション
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
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

void Enemy1::Finalize() {}
//
//void Enemy1::Shot(float speed)
//{
//    if (!player) return;
//    Vector2D dir = player->GetLocation() - location;
//    dir.Normalize();
//    auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
//    if (shot) shot->SetVelocity(dir * speed);
//}

void Enemy1::Shot(float speed)
{
    const int bullet_count = 3;
    const float angles[bullet_count] = { -15.0f, 0.0f, 15.0f }; // 発射角（度）

    for (int i = 0; i < bullet_count; ++i)
    {
        float angle_rad = angles[i] * DX_PI / 180.0f;
        Vector2D dir(sinf(angle_rad), cosf(angle_rad)); // Y方向が下

        auto bullet = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet2>(location);
        if (bullet)
        {
            bullet->SetVelocity(dir * speed);
            bullet->SetWaveParameters(30.0f, 5.0f); // 振幅30px、周波数5
        }
    }
}

//void Enemy1::SetPattern(Enemy1Pattern new_pattern)
//{
//    pattern = new_pattern;
//    pattern_timer = 0.0f;
//}
