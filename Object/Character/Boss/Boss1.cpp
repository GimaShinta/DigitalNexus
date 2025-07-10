// 修正済み Boss1.cpp（ワープや移動のX/Y範囲を調整済み）
#include "Boss1.h"
#include "../Player/Player.h"
#include "../../Bullet/EnemyBullet1.h"
#include "../../Item/PowerUp/PowerUp.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
constexpr float PI = 3.1415926f;


Boss1::Boss1()
{
    srand(static_cast<unsigned int>(time(nullptr)));
}

Boss1::~Boss1()
{}

void Boss1::Initialize()
{
    z_layer = 2;
    box_size = 35;
    hp = 3000;

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);

    is_mobility = true;
    start_location = location;
    is_returning = false;
    has_shot = false;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images_a = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy75_a_swapped.png", 6, 6, 1, 40, 40);
    images_b = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy75_b.png", 6, 6, 1, 40, 40);
    images = images_b;
    image = images[0];

    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255 * 100 / 100, sound_destroy);

    ChangePatternRandomly();

    floating_center_initialized = false;

    is_transforming = false;
    transform_timer = 0.0f;
    flash_timer = 0.0f;
    visible = true;

    life_timer = 0.0f;
}

void Boss1::Update(float delta_second)
{
    life_timer += delta_second;

    if (!is_returning && life_timer >= 40.0f)
    {
        is_returning = true;
        is_leaving = false;
        return_target = { 640.0f, 240.0f };
        original_location_before_return = location;
        return_timer = 0.0f;
        stop_timer = 0.0f;
    }

    if (pattern == BossPattern::Entrance && pattern_timer >= 5.0f)
        SetPattern(BossPattern::GlitchWarp);
    if (pattern == BossPattern::GlitchWarp && pattern_timer >= 4.0f)
        SetPattern(BossPattern::CircleMove);
    if (pattern == BossPattern::CircleMove && pattern_timer >= 6.0f)
        SetPattern(BossPattern::ZigzagMove);
    if (pattern == BossPattern::ZigzagMove && pattern_timer >= 5.0f)
        SetPattern(BossPattern::RandomMove);

    if (is_returning)
    {
        const float speed = 200.0f;

        if (!is_leaving)
        {
            return_timer += delta_second;
            float t = (return_timer / return_duration > 1.0f) ? 1.0f : return_timer / return_duration;
            location = original_location_before_return + (return_target - original_location_before_return) * t;
            velocity = { 0.0f, 0.0f };

            if (t >= 1.0f)
            {
                stop_timer += delta_second;
                if (stop_timer >= stop_duration)
                {
                    is_leaving = true;
                }
            }
        }
        else
        {
            velocity = { 0.0f, -speed };
            location += velocity * delta_second;

            if (location.y < -100.0f)
            {
                is_alive = false;
                is_destroy = true;
            }
        }
        return;
    }

    if (!is_transformed && hp < 500)
    {
        is_transforming = true;
        is_transformed = true;
        transform_timer = 0.0f;
        flash_timer = 0.3f;
        visible = true;
        is_flashing = true;
        is_screen_flash = true;
        screen_flash_timer = screen_flash_duration;
    }

    if (is_transforming)
    {
        transform_timer += delta_second;
        flash_timer += delta_second;

        if (flash_timer >= flash_interval)
        {
            flash_timer = 0.0f;
            visible = !visible;
        }

        velocity = { 0, 0 };

        if (transform_timer >= transform_duration)
        {
            is_transforming = false;
            visible = true;
            images = images_a;
            image = images[0];
        }

        return;
    }

    spawn_delay_timer -= delta_second;
    pattern_timer += delta_second;

    GameObjectBase::AnimationControl(delta_second, images, anim_indices, 10.0f);

    switch (pattern)
    {
    case BossPattern::Entrance:
    {
        const float move_down_duration = 3.0f;
        const float target_y = 150.0f;
        const float amplitude_x = 250.0f;
        const float frequency_x = 1.0f;
        const float amplitude_y = 50.0f;
        const float frequency_y = 2.0f;

        const float warp_times[5] = { 0.3f, 0.7f, 1.1f, 1.5f, 1.9f };

        for (int i = 0; i < 5; ++i)
        {
            if (!entrance_warp_done[i] && pattern_timer >= warp_times[i])
            {
                // Entrance
                location.x = 400.0f + GetRand(480); // X: 400?880
                location.y = 100.0f + GetRand(260); // Y: 100?360  // Y: 100～360 → 上半分だけに限定
                entrance_warp_done[i] = true;

                is_flashing = true;
                flash_timer = 0.1f;
                image = images_a[GetRand((int)images_a.size() - 1)];
            }
        }


        if (pattern_timer < move_down_duration && location.y < target_y)
        {
            velocity = { 0, 100 };
            location += velocity * delta_second;
            if (location.y > target_y) location.y = target_y;
        }
        else
        {
            if (!floating_center_initialized)
            {
                floating_center_x = location.x;
                floating_center_y = target_y;
                floating_center_initialized = true;
            }

            float t = pattern_timer - move_down_duration;
            location.x = floating_center_x + amplitude_x * sinf(t * frequency_x + 3.14f);
            location.y = floating_center_y + amplitude_y * sinf(t * frequency_y);
            velocity = { 0, 0 };
        }
        break;
    }

    case BossPattern::GlitchWarp:
    {
        static float warp_timer = 0.0f;
        warp_timer += delta_second;

        if (warp_timer >= 0.5f)
        {
            location.x = 400.0f + GetRand(480); // X: 400?880
            location.y = 100.0f + GetRand(260); // Y: 100?360

            warp_timer = 0.0f;

            image = images_a[GetRand((int)images_a.size() - 1)];
            is_flashing = true;
            flash_timer = 0.1f;
        }


        velocity = { 0, 0 };
        break;
    }
    case BossPattern::CircleMove:
    {
        float radius = 100.0f;
        float speed = 2.0f;

        // 円運動の中心を画面中央に固定
        if (!floating_center_initialized)
        {
            floating_center_x = 640.0f;
            floating_center_y = 300.0f;
            floating_center_initialized = true;
        }

        float t = pattern_timer * speed;
        location.x = floating_center_x + radius * cosf(t);
        location.y = floating_center_y + radius * sinf(t);

        velocity = { 0, 0 };
        break;
    }

    case BossPattern::ZigzagMove:
    {
        float amplitude = 100.0f;
        float frequency = 2.0f;
        float speedY = 50.0f;

        if (!floating_center_initialized)
        {
            floating_center_x = location.x;  // 現在の位置を中心に
            floating_center_y = location.y;
            floating_center_initialized = true;
        }

        location.x = floating_center_x + amplitude * sinf(pattern_timer * frequency);
        location.y += speedY * delta_second;

        velocity = { 0, speedY };
        break;
    }

    case BossPattern::RandomMove:
    {
        static float random_move_timer = 0.0f;
        static Vector2D current_velocity = { 0.0f, 0.0f };

        random_move_timer += delta_second;

        if (random_move_timer >= 1.0f)
        {
            float vx = (GetRand(200) - 100);
            float vy = (GetRand(120) - 60);
            current_velocity = { vx, vy };
            random_move_timer = 0.0f;
        }

        Vector2D next_pos = location + current_velocity * delta_second;

        if (next_pos.x < 300.0f || next_pos.x > 980.0f)
            current_velocity.x *= -1;
        if (next_pos.y < 10.0f || next_pos.y > 710.0f)
            current_velocity.y *= -1;

        location += current_velocity * delta_second;
        velocity = current_velocity;
        break;
    }
    }

    location += velocity * delta_second;

    if (is_flashing)
    {
        flash_timer -= delta_second;
        if (flash_timer <= 0.0f)
        {
            is_flashing = false;
            flash_timer = 0.0f;
        }
    }

    if (is_screen_flash)
    {
        screen_flash_timer -= delta_second;
        if (screen_flash_timer <= 0.0f)
        {
            is_screen_flash = false;
        }
    }

    if (hp <= 0)
    {
        is_alive = false;
        is_destroy = true;
        PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);
        // ====== 演出とスコア処理 ======
        auto* manager = Singleton<EffectManager>::GetInstance();
        int anim_id = manager->PlayerAnimation(EffectName::eExprotion, location, 0.05f, false);
        manager->SetScale(anim_id, 0.5f);

        Singleton<ScoreData>::GetInstance()->SetScoreData(1000);
    }


    Shot(delta_second);
    __super::Update(delta_second);

    // 範囲制限
    if (location.x < 300.0f) location.x = 300.0f;
    if (location.x > 980.0f) location.x = 980.0f;
    if (location.y < 10.0f) location.y = 10.0f;
    if (location.y > 710.0f) location.y = 710.0f;
}

void Boss1::Draw(const Vector2D& screen_offset) const
{
    float scale = 4.0f;

    if (pattern == BossPattern::Entrance && is_flashing)
    {
        scale += (GetRand(20) - 10) * 0.01f;
    }

    if (is_transforming)
    {
        if (visible)
        {
            SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
            DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    else
    {
        DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);
    }

    if (is_flashing && pattern == BossPattern::Entrance && pattern_timer <= 3.0f)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
        DrawCircle((int)location.x, (int)location.y, 60, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}


void Boss1::Finalize()
{}

void Boss1::Shot(float delta_second)
{
    if (!player) return;

    shot_timer += delta_second;

    if (shot_timer >= 0.5f)
    {
        Vector2D dir = player->GetLocation() - location;
        float len = dir.Length();
        if (len > 0) dir /= len;

        auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(location);
        shot->SetVelocity(dir);

        shot_timer = 0.0f;
    }
}

void Boss1::ChangePatternRandomly()
{}

bool Boss1::GetIsAlive() const
{
    return is_alive;
}

void Boss1::SetPattern(BossPattern new_pattern)
{
    pattern = new_pattern;
    pattern_timer = 0.0f;
    has_shot = false;
    after_shot_timer = 0.0f;
    floating_center_initialized = false;

    switch (pattern)
    {
    case BossPattern::Entrance:
        images = images_b;
        anim_indices = { 0, 1, 2, 3, 4, 5 };
        for (int i = 0; i < 5; ++i) entrance_warp_done[i] = false;  // 初期化
        break;
    case BossPattern::GlitchWarp:   anim_indices = { 0, 2, 4 }; break;
    case BossPattern::CircleMove:   anim_indices = { 1, 3, 5 }; break;
    case BossPattern::ZigzagMove:   anim_indices = { 2, 3, 4 }; break;
    case BossPattern::RandomMove:   anim_indices = { 2, 3, 4 }; break;
    }

    image = images[0];
}
