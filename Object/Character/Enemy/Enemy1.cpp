#include "Enemy1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

Enemy1::Enemy1()
{
}

Enemy1::Enemy1(const Vector2D& pos)
{
    location = pos;
}

Enemy1::~Enemy1()
{}

void Enemy1::Initialize()
{
    z_layer = 3;
    box_size = 12;
    hp = 20;

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);

    is_mobility = true;
    start_location = location;
    is_returning = false;
    has_shot = false;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images_a = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_a.png", 4, 4, 1, 32, 32);
    images_b = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_b.png", 12, 12, 1, 24, 24);

    images = images_a;
    image = images[0];

    //SE
   // sound_hit = rm->GetSounds("Resource/Sound/reaction.mp3");
    //sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    //se_start = rm->GetSounds("Resource/sound/se/effect/buon.mp3");
    //ChangeVolumeSoundMem(255 * 100 / 100, sound_destroy);
    //ChangeVolumeSoundMem(255 * 10 / 100, se_start);
    ChangePatternRandomly();
}

void Enemy1::Update(float delta_second)
{
    spawn_delay_timer -= delta_second;
    pattern_timer += delta_second;

    GameObjectBase::AnimationControl(delta_second, images, anim_indices, 10.0f);

    switch (pattern)
    {
    case Enemy1Pattern::MoveStraight:
    {
        const float appear_time = 0.8f;

        if (pattern_timer < appear_time)
        {
            float t = pattern_timer / appear_time;
            scale = 0.5f + t * 1.5f;
            velocity = { 0, 180 };
        }
        else
        {
            scale = 2.0f;
            velocity = { 0, 180 };
        }
        //Shot(delta_second);
        break;
    }

    case Enemy1Pattern::RightMove:
    case Enemy1Pattern::LeftMove:
    {
        const float arc_duration = 1.2f;
        const float decel_duration = 1.0f;
        const float accel_duration = 1.0f;

        if (pattern_timer < arc_duration)
        {
            float t = pattern_timer / arc_duration;
            float angle = (1.0f - t) * (3.1415f / 2.0f);
            float dir = (pattern == Enemy1Pattern::RightMove) ? 1.0f : -1.0f;

            velocity.x = cosf(angle) * 250.0f * dir;
            velocity.y = sinf(angle) * 250.0f;

            scale = 4.0f - 2.0f * t;
        }
        else if (pattern_timer < arc_duration + decel_duration)
        {
            float t = (pattern_timer - arc_duration) / decel_duration;
            velocity.x = 250.0f * (1.0f - t) * ((pattern == Enemy1Pattern::RightMove) ? 1.0f : -1.0f);
            velocity.y = 0.0f;
            scale = 2.0f;
        }
        else if (pattern_timer < arc_duration + decel_duration + accel_duration)
        {
            float t = (pattern_timer - arc_duration - decel_duration) / accel_duration;
            velocity.x = ((pattern == Enemy1Pattern::RightMove) ? 200.0f : -200.0f) + ((pattern == Enemy1Pattern::RightMove) ? 1 : -1) * 300.0f * t;
            velocity.y = 0.0f;
            scale = 2.0f;
        }
        else
        {
            velocity.x = (pattern == Enemy1Pattern::RightMove) ? 500.0f : -500.0f;
            velocity.y = 0.0f;
            scale = 2.0f;
        }

        Shot(delta_second);
        break;
    }

    case Enemy1Pattern::ZIgzag:
        velocity.x = sinf(pattern_timer * 1.5f) * 320;
        velocity.y = 100;
        break;

    case Enemy1Pattern::SideAppearAndShoot:
    {
        const float appear_duration = 1.0f;

        if (pattern_timer < appear_duration)
        {
            velocity = (location.x > 400) ? Vector2D(-200, 0) : Vector2D(200, 0);
        }
        else
        {
            velocity = { 0, 0 };
            Shot(delta_second);
        }
        break;
    }

    case Enemy1Pattern::MoveAndStopShoot:
        if (!has_shot)
        {
            if (location.y < 300)
            {
                scale = 2.0f;
                velocity = { 0, 300 };
            }
            else
            {
                scale = 2.0f;
                velocity = { 0, 0 };
                shot_timer += delta_second;

                if (shot_timer >= 2.0f && player)
                {
                    Shot(delta_second);
                    has_shot = true;
                    shot_timer = 0.0f;
                }
            }
        }
        else
        {
            velocity = { 0, 0 };
            after_shot_timer += delta_second;

            if (after_shot_timer >= 2.0f)
            {
                velocity = { 0, -150 };
            }
        }
        break;

    case Enemy1Pattern::MoveThenDiagonal:
        velocity = (pattern_timer < 1.0f) ? Vector2D(0, 100) : Vector2D(100, 100);
        break;

    case Enemy1Pattern::Formation:
        velocity.x = sinf((pattern_timer + start_location.y * 0.01f) * 1.5f) * 80.0f;
        velocity.y = 80.0f;
        break;

    case Enemy1Pattern::DiveOnce:
        if (pattern_timer < 1.0f)
        {
            scale = 2.0f;
            velocity = { 0, 300 };
        }
        else if (pattern_timer < 2.0f)
        {
            scale = 2.0f;
            velocity = { 0, 0 };
        }
        else
        {
            if (!has_shot && player)
            {
                Vector2D dir = player->GetLocation() - location;
                float len = dir.Length();
                if (len > 0) dir /= len;
                velocity = dir * 600.0f;
                has_shot = true;
            }
        }
        break;

    case Enemy1Pattern::ArcMoveAndStop:
    {
        const float stop_y = custom_stop_y;
        if (location.y < stop_y)
            velocity = { 0, 100 };
        else
        {
            velocity = { 0, 0 };
            if (pattern_timer > 1.0f)
            {
                Vector2D to_player = player->GetLocation() - location;
                to_player.Normalize();
                velocity = to_player * 250;
            }
        }
        break;
    }

    case Enemy1Pattern::DepthAppear:
    {
        const float appear_time = 1.0f;

        if (pattern_timer < appear_time)
        {
            float t = pattern_timer / appear_time;
            scale = 0.2f + t * 1.3f;
            velocity = { 0, 0 };
        }
        else
        {
            scale = 1.5f;
            velocity = { 0, 0 };
            Shot(delta_second);
        }
        break;
    }

    case Enemy1Pattern::RetreatUp:
    {
        const float decel_duration = 0.5f;
        const float pause_duration = 0.3f;
        const float accel_duration = 0.7f;

        if (pattern_timer < decel_duration)
        {
            float t = pattern_timer / decel_duration;
            velocity.y = 200.0f * (1.0f - t);
        }
        else if (pattern_timer < decel_duration + pause_duration)
        {
            velocity.y = 0.0f;
        }
        else
        {
            float t = (pattern_timer - decel_duration - pause_duration) / accel_duration;
            if (t > 1.0f) t = 1.0f;
            velocity.y = -100.0f - 400.0f * t;
        }

        if (location.y < -100)
        {
            is_destroy = true;
        }

        break;
    }

    case Enemy1Pattern::SlowDownThenMove:
    {
        const float decel_duration = 1.0f;
        const float accel_duration = 1.0f;

        if (pattern_timer < decel_duration)
        {
            float t = pattern_timer / decel_duration;
            velocity.y = 250.0f * (1.0f - t);
            scale = 1.0f + t * 0.5f;
        }
        else if (pattern_timer < decel_duration + accel_duration)
        {
            float t = (pattern_timer - decel_duration) / accel_duration;
            velocity.y = 100.0f + 200.0f * t;
            scale = 1.5f;
        }
        else
        {
            velocity.y = 300.0f;
            scale = 1.5f;
        }
        break;
    }

    case Enemy1Pattern::PauseThenRush:
    {
        const float descend_duration = 0.8f;
        const float pause_duration = 0.5f;

        if (pattern_timer < descend_duration)
        {
            velocity = { 0.0f, 150.0f };
        }
        else if (pattern_timer < descend_duration + pause_duration)
        {
            velocity = { 0.0f, 0.0f };
        }
        else
        {
            Vector2D to_player = player->GetLocation() - location;
            to_player.Normalize();
            velocity = to_player * 350.0f;
        }
        break;
    }

    case Enemy1Pattern::RotateAndShoot:
    {
        velocity = { 0.0f, 80.0f };

        if (fmod(pattern_timer, 0.5f) < delta_second && player)
        {
            const float base_angle = pattern_timer * 2.0f;
            for (int i = 0; i < 8; ++i)
            {
                float angle = base_angle + i * DX_PI / 4.0f;
                Vector2D dir(cosf(angle), sinf(angle));
                auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
                shot->SetVelocity(dir * 220.0f);
            }
        }
        break;
    }
    case Enemy1Pattern::SlowInShootOut:
    {
        const float decel_duration = 1.0f;
        const float shoot_duration = 0.5f;
        const float escape_duration = 1.0f;

        if (pattern_timer < decel_duration)
        {
            // 徐々に減速
            float t = pattern_timer / decel_duration;
            float vx = (start_location.x < 640.0f) ? 250.0f * (1.0f - t) : -250.0f * (1.0f - t);
            velocity = { vx, 0.0f };
        }
        else if (pattern_timer < decel_duration + shoot_duration)
        {
            // 停止して弾発射
            velocity = { 0.0f, 0.0f };
            Shot(delta_second);
        }
        else if (pattern_timer < decel_duration + shoot_duration + escape_duration)
        {
            // 加速して退場
            float t = (pattern_timer - decel_duration - shoot_duration) / escape_duration;
            float vx = (start_location.x < 640.0f) ? 150.0f + 350.0f * t : -150.0f - 350.0f * t;
            velocity = { vx, 0.0f };
        }
        else
        {
            // 完全退場スピード
            velocity.x = (start_location.x < 640.0f) ? 500.0f : -500.0f;
            velocity.y = 0.0f;
        }
        break;
    }
    }

    // 座標更新
    location += velocity * delta_second;

    // 当たり判定の有効・無効切り替え（scaleが1.5以上で有効）
    collision.is_blocking = (scale >= 1.5f);

    // HPが0以下で爆発演出＋破壊
    if (hp <= 0)
    {
        EffectManager* am = Singleton<EffectManager>::GetInstance();
        SEManager* sm = Singleton<SEManager>::GetInstance();
        sm->PlaySE(SE_NAME::Destroy);
        DropItems();

        int anim_id = am->PlayerAnimation(EffectName::eExprotion2, location, 0.03f, false);
        am->SetScale(anim_id, 0.5f);

        Singleton<ScoreData>::GetInstance()->AddScore(GetRand(100) + 200);
        is_destroy = true;
    }

    __super::Update(delta_second);
}


void Enemy1::Draw(const Vector2D& screen_offset) const
{
    DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
}

void Enemy1::Finalize()
{}

void Enemy1::Shot(float delta_second)
{
    shot_timer += delta_second;

    if (shot_timer >= 2.0f && player)
    {
        Vector2D dir = player->GetLocation() - location;
        float len = dir.Length();
        if (len > 0) dir /= len;

        auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
        shot->SetVelocity(dir);

        shot_timer = 0.0f;
    }
}

void Enemy1::ChangePatternRandomly()
{
    int r = rand() % static_cast<int>(Enemy1Pattern::Count);
    SetPattern(static_cast<Enemy1Pattern>(r));
}

void Enemy1::SetPattern(Enemy1Pattern new_pattern)
{
    pattern = new_pattern;
    pattern_timer = 0.0f;
    has_shot = false;
    after_shot_timer = 0.0f;

    switch (pattern)
    {
    case Enemy1Pattern::MoveStraight:
        hp = 10;
        images = images_b;
        anim_indices = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        scale = 1.5f;
        break;

    case Enemy1Pattern::RightMove:
    case Enemy1Pattern::LeftMove:
        //PlaySoundMem(se_start, DX_PLAYTYPE_BACK);
        hp = 20;
        images = images_a;
        anim_indices = { 0, 1, 2, 3 };
        scale = 1.5f;
        break;

    case Enemy1Pattern::ZIgzag:
    case Enemy1Pattern::MoveThenDiagonal:
        hp = 10;
        images = images_b;
        anim_indices = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        scale = 1.5f;
        break;

    case Enemy1Pattern::MoveAndStopShoot:
    case Enemy1Pattern::Formation:
    case Enemy1Pattern::DiveOnce:
        hp = 20;
        images = images_a;
        anim_indices = { 0, 1, 2, 3 };
        scale = 1.5f;
        break;

    case Enemy1Pattern::SideAppearAndShoot:
    case Enemy1Pattern::ArcMoveAndStop:
        hp = 20;
        images = images_b;
        anim_indices = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        scale = 1.5f;
        break;

    case Enemy1Pattern::DepthAppear:
        hp = 20;
        images = images_b;
        anim_indices = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        scale = 0.2f;
        break;
    case Enemy1Pattern::RetreatUp:
        hp = 10;
        images = images_a;
        anim_indices = { 0, 1, 2, 3 };
        scale = 1.5f;
        break;
    case Enemy1Pattern::RotateAndShoot:
        hp = 30;
        images = images_a;
        anim_indices = { 0, 1, 2, 3 };
        scale = 1.5f;
        break;
    case Enemy1Pattern::SlowInShootOut:
        hp = 25;
        images = images_b;
        anim_indices = { 0,1,2,3,4,5,6,7,8,9,10,11 };
        scale = 1.5f;
        break;
    }

    image = images[0];
}