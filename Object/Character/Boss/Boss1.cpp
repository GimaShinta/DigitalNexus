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

    // �Փːݒ�
    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);
}

void Boss1::Update(float delta_second)
{
    pattern_timer += delta_second;

    // ���j����
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

    // ��Ԃ��Ƃ̋���
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

    // �A�j���[�V�����X�V (Dead�ȊO)
    if (pattern != BossPattern::Dead)
    {
        int frame = static_cast<int>((GetNowCount() / 10) % images.size());
        image = images[frame];
    }
}

void Boss1::UpdateEntrance(float delta_second)
{
    // y�������C�[�W���O�ňړ�
    float speed = 3.0f; // �傫���قǑ������B
    location.y += (target_pos.y - location.y) * speed * delta_second;

    // ���B����
    if (fabs(location.y - target_pos.y) < 1.0f)
    {
        location.y = target_pos.y;
        pattern = BossPattern::Hovering;
        shot_timer = 0.0f;
        move_timer = 0.0f;
    }

    // �t�F�[�h�C��
    alpha += delta_second * 150.0f;
    if (alpha > 255.0f) alpha = 255.0f;
}

void Boss1::UpdateHovering(float delta_second)
{
    move_timer += delta_second;

    // �������L�߂� + �c�������h���
    float amplitudeX = 130.0f;
    float amplitudeY = 25.0f;
    float speed = 1.2f;

    location.x = D_WIN_MAX_X / 2 + sin(move_timer * speed) * amplitudeX;
    location.y = target_pos.y + sin(move_timer * speed * 2.0f) * amplitudeY;

    // �U���Ԋu��Z�� & �����_��
    shot_timer += delta_second;
    static float next_shot_interval = 0.5f;

    if (shot_timer > next_shot_interval)
    {
        switch (attack_mode)
        {
        case 0: ShotSpiral(delta_second); break;
        case 1: ShotAllRange(); break;
        case 2: ShotCrossShot(); break;
        case 3: ShotFanWide(); break;        // ���V�����e���p�^�[���ɒu������
        case 4: ShotWaveBullets(); break;
        case 5: ShotTripleSpread(); break;
        case 6: ShotFastSpiral(delta_second); break;
        }
        attack_mode = (attack_mode + 1) % 7;

        // 0.6?1.0�b�̃����_���Ԋu�ōU��
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

    // ���ɉ������ւ΂�܂�
    const int bullet_count = 24;
    const float start_angle = -60.0f;   // �������̊p�x
    const float angle_step = 5.0f;      // �p�x�̍L���蕝

    for (int i = 0; i < bullet_count; i++)
    {
        // ����������ɐ��ɍL����
        float rad = (90.0f + start_angle + i * angle_step) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 300.0f, sin(rad) * 300.0f);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }

    // �����тɔh���SE
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}



void Boss1::ShotWaveBullets()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // �����������5�����֌���
    for (int i = 0; i < 5; i++)
    {
        float rad = (90.0f + ((i * 20.0f) - 40.0f)) * DX_PI / 180.0f; // �������
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
        Vector2D vel(dir * 120.0f, 400.0f); // ���E�����E�E
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
    angle += 25.0f; // ������]
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
        // �����ӏ���������
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
