#include "Boss1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ShakeManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../Character/Player/Player.h"
#include "../../Bullet/EnemyBullet/EnemyBullet3.h"

Boss1::Boss1() {}
Boss1::~Boss1() {}

void Boss1::Initialize()
{
    box_size = Vector2D(35, 35);
    hp = 3000;
    is_alive = true;
    pattern = BossPattern::Entrance;

    location = Vector2D(D_WIN_MAX_X / 2, -250);
    target_pos = Vector2D(D_WIN_MAX_X / 2, 180);
    velocity = Vector2D(0, 0);

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    //images = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy75_b.png", 6, 6, 1, 40, 40);
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy87_a.png", 6, 6, 1, 56, 56);
    image = images[0];

    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255, sound_destroy);

    alpha = 0.0f;
    pattern_timer = 0.0f;

    // 衝突設定
    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);
}

void Boss1::Update(float delta_second)
{
    pattern_timer += delta_second;

    // 撃破判定
    if (hp <= 0 && pattern != BossPattern::Dead)
    {
        pattern = BossPattern::Dead;
        is_alive = false;
        explosion_timer = 0.0f;
        explosions_started = false;
        explosion_count = 0;
        final_explosion_done = false;

        PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);
        Singleton<ShakeManager>::GetInstance()->StartShake(1.0f, 30.0f, 30.0f);
    }

    // 状態ごとの挙動
    switch (pattern)
    {
    case BossPattern::Entrance:
        UpdateEntrance(delta_second);
        break;

    case BossPattern::Hovering:
        UpdateHovering(delta_second);
        break;

    case BossPattern::Dead:
        ExplosionEffect(delta_second);
        break;
    }

    // アニメーション更新 (Dead以外)
    if (pattern != BossPattern::Dead)
    {
        int frame = static_cast<int>((GetNowCount() / 10) % images.size());
        image = images[frame];
    }
}

void Boss1::UpdateEntrance(float delta_second)
{
    // y方向をイージングで移動
    float speed = 3.0f; // 大きいほど早く到達
    location.y += (target_pos.y - location.y) * speed * delta_second;

    // 到達判定
    if (fabs(location.y - target_pos.y) < 1.0f)
    {
        location.y = target_pos.y;
        pattern = BossPattern::Hovering;
        shot_timer = 0.0f;
        move_timer = 0.0f;
    }

    // フェードイン
    alpha += delta_second * 150.0f;
    if (alpha > 255.0f) alpha = 255.0f;
}

void Boss1::UpdateHovering(float delta_second)
{
    move_timer += delta_second;

    // 横幅を広めに + 縦も少し揺れる
    float amplitudeX = 130.0f;
    float amplitudeY = 25.0f;
    float speed = 1.2f;

    location.x = D_WIN_MAX_X / 2 + sin(move_timer * speed) * amplitudeX;
    location.y = target_pos.y + sin(move_timer * speed * 2.0f) * amplitudeY;

    // 攻撃間隔を短め & ランダム
    shot_timer += delta_second;
    static float next_shot_interval = 0.5f;

    if (shot_timer > next_shot_interval)
    {
        switch (attack_mode)
        {
        case 0: ShotSpiral(delta_second); break;
        case 1: ShotAllRange(); break;
        case 2: ShotCrossShot(); break;
        case 3: ShotFanWide(); break;        // ★新しい弾幕パターンに置き換え
        case 4: ShotWaveBullets(); break;
        case 5: ShotTripleSpread(); break;
        case 6: ShotFastSpiral(delta_second); break;
        }
        attack_mode = (attack_mode + 1) % 7;

        // 0.6?1.0秒のランダム間隔で攻撃
        next_shot_interval = 0.6f + (GetRand(40) / 100.0f);
        shot_timer = 0.0f;
    }
}


void Boss1::ShotSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f;

    for (int dir = -1; dir <= 1; dir += 2)
    {
        for (int i = 0; i < 6; i++)
        {
            float rad = (angle + dir * i * 60.0f) * DX_PI / 180.0f;
            Vector2D vel(cos(rad) * 300.0f, sin(rad) * 300.0f);
            auto bullet = objm->CreateObject<EnemyBullet3>(location);
            bullet->SetVelocity(vel);
        }
    }
    angle += 15.0f;
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotAllRange()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 24; i++)
    {
        float rad = (i * 15.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 280.0f, sin(rad) * 280.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotCrossShot()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 4; i++)
    {
        float rad = (i * 90.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 350.0f, sin(rad) * 350.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotFanWide()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 扇状に下方向へばらまく
    const int bullet_count = 24;
    const float start_angle = -60.0f;   // 左下側の角度
    const float angle_step = 5.0f;      // 角度の広がり幅

    for (int i = 0; i < bullet_count; i++)
    {
        // 下方向を基準に扇状に広げる
        float rad = (90.0f + start_angle + i * angle_step) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 300.0f, sin(rad) * 300.0f);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }

    // 撃つたびに派手なSE
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}



void Boss1::ShotWaveBullets()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 下方向を基準に5方向へ撃つ
    for (int i = 0; i < 5; i++)
    {
        float rad = (90.0f + ((i * 20.0f) - 40.0f)) * DX_PI / 180.0f; // 下方向基準
        Vector2D vel(cos(rad) * 250.0f, sin(rad) * 250.0f);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }

    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}



void Boss1::ShotTripleSpread()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int dir = -1; dir <= 1; dir++)
    {
        Vector2D vel(dir * 120.0f, 400.0f); // 左・中央・右
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotFastSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f;

    for (int i = 0; i < 12; i++)
    {
        float rad = (angle + i * 30.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 380.0f, sin(rad) * 380.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    angle += 25.0f; // 高速回転
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ExplosionEffect(float delta_second)
{
    explosion_timer += delta_second;

    if (!explosions_started)
    {
        Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.06f, false);
        explosions_started = true;
    }
    else if (explosion_timer > 0.1f && explosion_count <= 10)
    {
        // 複数箇所同時爆発
        for (int i = 0; i < 3; i++)
        {
            Vector2D randPos = location + Vector2D(GetRand(180) - 90, GetRand(180) - 90);
            Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, randPos, 0.05f, false);
        }
        explosion_count++;
        explosion_timer = 0.0f;
    }

    if (explosion_count > 10)
    {
        if (!final_explosion_done)
        {
            Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.08f, false);
            final_explosion_done = true;
            explosion_timer = 0.0f;
        }
        else if (explosion_timer > 0.2f)
        {
            Singleton<ScoreData>::GetInstance()->AddScore(15000);
            is_destroy = true;
        }
    }
}

void Boss1::Draw(const Vector2D& screen_offset) const
{
    if (pattern != BossPattern::Dead)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)alpha);
        DrawRotaGraph(location.x, location.y, 3.5f, 0.0f, image, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Boss1::Finalize() {}

void Boss1::SetPattern(BossPattern new_pattern)
{
    pattern = new_pattern;
    pattern_timer = 0.0f;
}

bool Boss1::GetIsAlive() const
{
    return is_alive;
}
