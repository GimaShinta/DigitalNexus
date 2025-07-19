#include "Enemy2.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet3.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include <cmath>

Enemy2::Enemy2()
{}
Enemy2::Enemy2(const Vector2D& pos)
{
    location = pos;
}
Enemy2::~Enemy2()
{}

void Enemy2::Initialize()
{
    z_layer = 3;
    box_size = Vector2D(120, 40);
    hp = 5000;

    phase = Enemy2Phase::Appearing;
    appear_timer = 0.0f;
    is_invincible = true;
    scale = 4.0f;
    alpha = 0;
    velocity = Vector2D(0, 0);

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);
    is_mobility = true;

    auto* rm = Singleton<ResourceManager>::GetInstance();
    image = rm->GetImages("Resource/Image/Object/Enemy/Zako5/enemy53.png")[0];

    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255 * 100 / 100, sound_destroy);
}

void Enemy2::Update(float delta)
{
    switch (phase)
    {
    case Enemy2Phase::Appearing:
    {
        appear_timer += delta;

        if (velocity.x == 0.0f && velocity.y == 0.0f)
        {
            velocity = (location.x < 640.0f)
                ? Vector2D(0.0f, 150.0f)   // 左から登場 → 下へ
                : Vector2D(0.0f, -150.0f); // 右から登場 → 上へ
        }

        float t = (appear_timer < 1.0f) ? appear_timer : 1.0f;
        scale = 4.0f - (2.5f * t);
        location += velocity * delta;

        if (appear_timer >= 2.0f)
        {
            phase = Enemy2Phase::Fighting;
            is_invincible = false;
            scale = 1.5f;

            if (is_rectangular_loop_move && is_clockwise)
            {
                if (velocity.y > 0.0f) // 下へ
                {
                    velocity = Vector2D(250.0f, 0.0f);
                    move_state = MoveState::MoveRight;
                }
                else // 上へ
                {
                    velocity = Vector2D(0.0f, -150.0f);
                    move_state = MoveState::MoveUp;
                }
            }
            else if (is_rectangular_loop_move && !is_clockwise)
            {
                if (velocity.y > 0.0f) // 左から来た → 下から開始
                {
                    velocity = Vector2D(250.0f, 0.0f);  // →
                    move_state = MoveState::MoveRight;
                }
                else // 右から来た → 上から開始
                {
                    velocity = Vector2D(-250.0f, 0.0f); // ←
                    move_state = MoveState::MoveLeft;
                }
            }
        }
        break;
    }

    case Enemy2Phase::Fighting:
        if (is_rectangular_loop_move)
        {
            if (is_clockwise)
            {
                // 時計回り（↑ ← ↓ →）
                switch (move_state)
                {
                case MoveState::MoveUp:
                    location.y += velocity.y * delta;
                    if (location.y <= LOOP_TOP)
                    {
                        location.y = LOOP_TOP;
                        velocity = Vector2D(-250.0f, 0.0f);
                        move_state = MoveState::MoveLeft;
                    }
                    break;
                case MoveState::MoveLeft:
                    location.x += velocity.x * delta;
                    if (location.x <= LOOP_LEFT)
                    {
                        location.x = LOOP_LEFT;
                        velocity = Vector2D(0.0f, 150.0f);
                        move_state = MoveState::MoveDown;
                    }
                    break;
                case MoveState::MoveDown:
                    location.y += velocity.y * delta;
                    if (location.y >= LOOP_BOTTOM)
                    {
                        location.y = LOOP_BOTTOM;
                        velocity = Vector2D(250.0f, 0.0f);
                        move_state = MoveState::MoveRight;
                    }
                    break;
                case MoveState::MoveRight:
                    location.x += velocity.x * delta;
                    if (location.x >= LOOP_RIGHT)
                    {
                        location.x = LOOP_RIGHT;
                        velocity = Vector2D(0.0f, -150.0f);
                        move_state = MoveState::MoveUp;
                    }
                    break;
                }
            }
            else
            {
                // 反時計回り（← ↓ → ↑）→ 見た目は対称
                switch (move_state)
                {
                case MoveState::MoveLeft:
                    location.x += velocity.x * delta;
                    if (location.x <= LOOP_LEFT)
                    {
                        location.x = LOOP_LEFT;
                        velocity = Vector2D(0.0f, 150.0f);  // ↓
                        move_state = MoveState::MoveDown;
                    }
                    break;
                case MoveState::MoveDown:
                    location.y += velocity.y * delta;
                    if (location.y >= LOOP_BOTTOM)
                    {
                        location.y = LOOP_BOTTOM;
                        velocity = Vector2D(250.0f, 0.0f);  // →
                        move_state = MoveState::MoveRight;
                    }
                    break;
                case MoveState::MoveRight:
                    location.x += velocity.x * delta;
                    if (location.x >= LOOP_RIGHT)
                    {
                        location.x = LOOP_RIGHT;
                        velocity = Vector2D(0.0f, -150.0f);  // ↑
                        move_state = MoveState::MoveUp;
                    }
                    break;
                case MoveState::MoveUp:
                    location.y += velocity.y * delta;
                    if (location.y <= LOOP_TOP)
                    {
                        location.y = LOOP_TOP;
                        velocity = Vector2D(-250.0f, 0.0f);  // ←
                        move_state = MoveState::MoveLeft;
                    }
                    break;
                }
            }
        }

        Shot(delta);

        if (hp <= 0)
        {
            is_destroy = true;
            PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);
            auto* manager = Singleton<EffectManager>::GetInstance();
            int anim_id = manager->PlayerAnimation(EffectName::eExprotion2, location, 0.03f, false);
            manager->SetScale(anim_id, 1.5f);
            Singleton<ScoreData>::GetInstance()->AddScore(1500);
            Singleton<ShakeManager>::GetInstance()->StartShake(1.0, 5, 5);
        }
        break;
    }

    SetLocation(location);
    __super::Update(delta);
}




void Enemy2::EnableLoopMoveNow()
{
    loop_move_enabled = true;
}


void Enemy2::Shot(float delta)
{
    if (!is_attacking)
    {
        attack_cooldown -= delta;
        if (attack_cooldown <= 0.0f)
        {
            is_attacking = true;
            attack_cooldown = 4.0f;
            spiral_timer = 0.0f;
            spiral_total_time = 0.0f;
        }
    }

    if (is_attacking)
    {
        if (attack_pattern == 6) Pattern6(delta);
        else if (attack_pattern == 7) Pattern7(delta);

        if (spiral_total_time >= 2.5f)
        {
            is_attacking = false;
            attack_pattern = (attack_pattern == 6) ? 7 : 6;
        }
    }
}

void Enemy2::Pattern6(float delta)
{
    spiral_timer += delta;
    spiral_total_time += delta;

    if (spiral_timer >= 1.6f)
    {
        spiral_timer = 0.0f;
        const int bullet_num = 5;
        const float base_angle = 90.0f;
        const float fan_range = 120.0f;
        const float speed = 180.0f;

        for (int i = 0; i < bullet_num; ++i)
        {
            float angle = base_angle - (fan_range / 2) + (fan_range / (bullet_num - 1)) * i;
            float rad = angle * DX_PI / 180.0f;
            Vector2D vel(cos(rad) * speed, sin(rad) * speed);

            auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet3>(location);
            shot->SetVelocity(vel);
            shot->SetAttackPattrn(1);
        }
    }
}

void Enemy2::Pattern7(float delta)
{
    spiral_timer += delta;
    spiral_total_time += delta;

    if (spiral_timer >= 1.6f)
    {
        spiral_timer = 0.0f;
        const int bullet_num = 5;
        const float base_angle = 90.0f;
        const float fan_range = 60.0f;
        const float speed = 160.0f;

        for (int i = 0; i < bullet_num; ++i)
        {
            float angle = base_angle - (fan_range / 2) + (fan_range / (bullet_num - 1)) * i;
            float rad = angle * DX_PI / 180.0f;
            Vector2D vel(cos(rad) * speed, sin(rad) * speed);

            auto shot = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet2>(location);
            shot->SetVelocity(vel);
        }
    }
}

void Enemy2::Draw(const Vector2D& screen_offset) const
{
    DrawRotaGraph(location.x, location.y, scale, 3.14, image, TRUE);
}

void Enemy2::Finalize()
{}

void Enemy2::EnableCircularMove(const Vector2D& center, float radius, float speed, float start_angle)
{
    circular_center = center;
    circular_radius = radius;
    circular_speed = speed;
    circular_angle = start_angle;
    is_circular_move = true;
}

void Enemy2::EnableRectangularLoopMove(bool clockwise)
{
    is_rectangular_loop_move = true;
    is_clockwise = clockwise;
}
