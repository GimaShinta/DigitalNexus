#include "Enemy4.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include <cmath>

Enemy4::Enemy4() {}
Enemy4::~Enemy4() {}

void Enemy4::Initialize() {
    EnemyBase::Initialize();

    // �傫�߂Ɂi�����蔻����g��j
    hp = 3000;
    is_destroy = false;
    is_alive = true;
    box_size = Vector2D(50, 50);     // �����ԑ傫��
    z_layer = 2;

    scale_from = 4.0f;   // �� �f�J��
    scale_to = 3.0f;   // �� �퓬����3�{�ňێ�
    scale_draw = scale_from;

    // �摜�i�K�v�ɉ��������ւ��j
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy75_a.png",
        6, 6, 1, 40, 40);
    image_index = 0;
    image = images.empty() ? -1 : images[0];

    // ��ʎ�O�i��[�̂���Ɏ�O�j����g�����Ă���h
    enter_start = Vector2D(D_WIN_MAX_X * 0.5f, -240.0f);
    target_pos = Vector2D(D_WIN_MAX_X * 0.5f, D_WIN_MAX_Y * 0.32f);
    location = enter_start;

    alpha = 0;
    scale_draw = scale_from;        // 2.0 ����J�n�i�f�J���j

    // �o�ꒆ�͔�e�Ȃ� �� �퓬�ڍs���ɗL�����iBoss1 �Ɠ����l�����j:contentReference[oaicite:3]{index=3}L18-L27
    collision.is_blocking = false;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.clear();

    // �^�C�}�[�n
    pattern = Enemy4Pattern::Entrance;
    pattern_timer = 0.0f;
    shot_timer = 0.0f;

    wander_timer = 0.0f;
    next_wander = 1.2f;
    noise_t = 0.0f;
    attack_mode = 0;
}

void Enemy4::Update(float dt) {
    pattern_timer += dt;

    switch (pattern) {
    case Enemy4Pattern::Entrance: UpdateEntrance(dt); break;
    case Enemy4Pattern::Battle:   UpdateBattle(dt);   break;
    case Enemy4Pattern::Retreat:  UpdateRetreat(dt);  break;
    case Enemy4Pattern::Dead:     UpdateDead(dt);     break;
    }

    // �摜�A�j���[�V����
    if (!images.empty()) {
        anim_time += dt;
        if (anim_time >= 0.09f) {
            anim_time -= 0.09f;
            image_index = (image_index + 1) % (int)images.size();
            image = images[image_index];
        }
    }

    on_hit = false;
    EnemyBase::Update(dt);
}

void Enemy4::UpdateEntrance(float dt) {
    float t = Saturate(pattern_timer / enter_time);
    float e = EaseOutCubic(t);

    // �ʒu�F�ォ��ڕW��
    location.y = enter_start.y + (target_pos.y - enter_start.y) * e;

    // �X�P�[���F2.0 �� 1.2�i�g��O���牜�ցh�̉��ߊ��j
    scale_draw = scale_from + (scale_to - scale_from) * e;

    // �t�F�[�h�F0 �� 255
    alpha = (int)(255.0f * e);
    if (alpha > 255) alpha = 255;

    if (t >= 1.0f) {
        // �����蔻���L�����iBoss1 �Ɠ����v�z�j:contentReference[oaicite:4]{index=4}L18-L27
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type.clear();
        collision.hit_object_type.push_back(eObjectType::eAttackShot);
        collision.hit_object_type.push_back(eObjectType::eBeam);

        // �o�g����
        pattern = Enemy4Pattern::Battle;
        pattern_timer = 0.0f;
        shot_timer = 0.0f;

        // �p�j�̏����ڕW
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(200) - 100),
            D_WIN_MAX_Y * 0.28f + (GetRand(140) - 70)
        );
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f); // 1.0?1.8�b
    }
}

void Enemy4::UpdateBattle(float dt) {
    // ===== �s�K���p�j�i�����_���ڕW�փX���[�Y�Ǐ]�{�m�C�Y�h��j=====
    wander_timer += dt;
    noise_t += dt;

    if (wander_timer >= next_wander) {
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f); // 1.0?1.8�b
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(280) - 140),          // ���ɑ傫��
            D_WIN_MAX_Y * 0.28f + (GetRand(180) - 90)           // �c�������_��
        );
    }

    // �ڕW�ɃX���[�Y�Ǐ]�i�C�[�W���O�j
    Vector2D to = wander_target - location;
    location += to * (2.8f * dt); // �Ǐ]�W���i�����j

    // �����݃m�C�Y�i�����ڂ̕s�K�����j
    float nX = sinf(noise_t * 2.3f) * 6.0f + sinf(noise_t * 1.7f + DX_PI / 3) * 4.0f;
    float nY = cosf(noise_t * 2.0f) * 5.0f + sinf(noise_t * 2.6f + DX_PI / 5) * 3.0f;
    location.x += nX * dt * 4.0f;
    location.y += nY * dt * 4.0f;

    // ===== �U���i�����߃��[�e�[�V�����j=====
    shot_timer += dt;

    // HP�����i�U�����x���ρj
    float hp_ratio = (float)hp / 2300.0f;
    if (hp_ratio < 0.0f) hp_ratio = 0.0f;
    float interval = 0.95f * hp_ratio + 0.35f; // HP���ŒZ���i0.35?1.3s�j

    if (shot_timer >= interval) {
        shot_timer = 0.0f;

        switch (attack_mode % 4) {
        case 0:  ShotFanBurst();   break;   // �L�p3�A
        case 1:  ShotFastSpiral(); break;   // �����X�p�C����
        case 2:  ShotAllRange();   break;   // �S���ʃ����O
        default: ShotCrossFan();   break;   // �\���{��
        }
        attack_mode++;
    }

    // ���j
    if (hp <= 0) {
        pattern = Enemy4Pattern::Dead;
        pattern_timer = 0.0f;
    }
}

void Enemy4::ForceRetreat() {
    if (pattern == Enemy4Pattern::Dead || pattern == Enemy4Pattern::Retreat) return;

    // ���G���E��e�s��
    collision.is_blocking = false;
    collision.hit_object_type.clear();

    // �ޏ��
    pattern = Enemy4Pattern::Retreat;
    pattern_timer = 0.0f;
}

void Enemy4::UpdateRetreat(float dt) {
    // ������փX�b�Ƒޏ�i�����������j
    float up_speed = 260.0f + 120.0f * (pattern_timer); // ���X�ɉ���
    location.y -= up_speed * dt;

    // �t�F�[�h�A�E�g�i1.2�b���x�œ����Ɂj
    int a = 255 - (int)(pattern_timer * (255.0f / 1.2f));
    if (a < 0) a = 0;
    alpha = a;

    // ��ʊO�ɔ����������
    if (location.y < -120.0f || alpha == 0) {
        is_destroy = true;
    }
}

void Enemy4::UpdateDead(float /*dt*/) {
    Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.06f, false);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Destroy);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Dead1);
    Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
    Singleton<ScoreData>::GetInstance()->AddScore(1500);
    is_destroy = true;
}

// ===== �U���p�^�[�� =====
// Boss1 �̔��z�i�X�p�C���� / �S���� / ��j�� EnemyBullet2 �ōč\��:contentReference[oaicite:5]{index=5}L112-L140:contentReference[oaicite:6]{index=6}L142-L161:contentReference[oaicite:7]{index=7}L163-L190
void Enemy4::ShotFanBurst() {
    // �������L�p��3�A���i���Ԋu�j
    auto objm = Singleton<GameObjectManager>::GetInstance();
    const int volley = 3;
    const int count_per = 11;
    const float spread = 80.0f; // �L�p
    const float base_deg = 90.0f;
    const float gap = (count_per > 1) ? (spread / (count_per - 1)) : 0.0f;
    const float start = base_deg - spread * 0.5f;

    for (int v = 0; v < volley; ++v) {
        for (int i = 0; i < count_per; ++i) {
            float deg = start + gap * i;
            float rad = Deg2Rad(deg);
            Vector2D vel(std::cos(rad) * 320.0f, std::sin(rad) * 320.0f);
            auto b = objm->CreateObject<EnemyBullet2>(location);
            if (b) b->SetVelocity(vel);
        }
        // �A�˂̊Ԋu��Z�������邽�߁A�������������o��
        location.y += 2.0f;
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::ShotFastSpiral() {
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float base = 0.0f;
    const int ring = 14;
    const float gap = 360.0f / ring;

    for (int i = 0; i < ring; ++i) {
        float rad = Deg2Rad(base + i * gap);
        Vector2D vel(std::cos(rad) * 360.0f, std::sin(rad) * 360.0f);
        auto b = objm->CreateObject<EnemyBullet2>(location);
        if (b) b->SetVelocity(vel);
    }
    base += 28.0f; // ������]
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::ShotAllRange() {
    auto objm = Singleton<GameObjectManager>::GetInstance();
    const int count = 28; // �S���ʂ΂�܂�
    for (int i = 0; i < count; ++i) {
        float rad = Deg2Rad(i * (360.0f / count));
        Vector2D vel(std::cos(rad) * 300.0f, std::sin(rad) * 300.0f);
        auto b = objm->CreateObject<EnemyBullet2>(location);
        if (b) b->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::ShotCrossFan() {
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // �\���̂݁i�� ��̒ǉ����˂͍폜�j
    for (int i = 0; i < 4; ++i) {
        float rad = Deg2Rad(i * 90.0f);
        Vector2D vel(std::cos(rad) * 340.0f, std::sin(rad) * 340.0f);
        auto b = objm->CreateObject<EnemyBullet2>(location);
        if (b) b->SetVelocity(vel);
    }

    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::OnHitCollision(GameObjectBase* hit_object)
{
    auto type = hit_object->GetCollision().object_type;

    // �ʏ�e
    if (type == eObjectType::eAttackShot)
    {
        hp -= 10;
    }

    // �r�[���i�ڐG���A���Ԋu�Ō��Z�j
    if (type == eObjectType::eBeam)
    {
        beam_damage_timer += 1.0f / 60.0f; // 1�t���[��=1/60�b���Z
        if (beam_damage_timer >= 0.35f)
        {
            hp -= 10;
            beam_damage_timer = 0.0f;
        }
    }
}


void Enemy4::Draw(const Vector2D& screen_offset) const {
    if (image < 0) return;

    // ===== �������̑��x���i�X�g���[�N�j�Łg������h������ =====
    if (pattern == Enemy4Pattern::Entrance) {
        float t = Saturate(pattern_timer / enter_time);
        int streaks = 8;
        int a = (int)(120.0f * (1.0f - t)); // �ŏ��������߂�

        SetDrawBlendMode(DX_BLENDMODE_ADD, a);
        for (int i = 0; i < streaks; ++i) {
            float ox = (GetRand(120) - 60) * (1.0f - t) * 0.6f;
            int x0 = (int)(location.x - screen_offset.x + ox);
            int y0 = (int)(location.y - screen_offset.y - 40);
            int y1 = (int)(location.y - screen_offset.y - 6);
            DrawLine(x0, y0, x0, y1, GetColor(120, 200, 255));
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // �{��
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawRotaGraph(
        (int)(location.x - screen_offset.x),
        (int)(location.y - screen_offset.y),
        scale_draw,    // �� ���� 3.0f?4.0f �ɂȂ�
        0.0f,
        image,
        TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // === ���^HP�Q�[�W�i���{�X�Ǐ]�j ===
    {
        // hp��i0.0�`1.0�j
        float hp_ratio = (float)hp / 3000.0f;
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;
        if (hp_ratio > 1.0f) hp_ratio = 1.0f;

        // �t�F�[�h�W���i�o�꒼��͏����T���߁j
        float fade_ratio = (pattern_timer < 0.0f) ? 0.0f
            : ((pattern_timer > 1.2f) ? 1.0f : (pattern_timer / 1.2f));
        int bar_alpha = (int)(fade_ratio * 200);

        // �ʒu�E�T�C�Y�i�G�̓���j
        const int bar_width = 140;
        const int bar_height = 5;
        const int bar_x = (int)(location.x - screen_offset.x) - bar_width / 2;
        const int bar_y = (int)(location.y - screen_offset.y) - (int)(scale_draw * 24.0f) - 8;

        // �킸���ȗh�炬�i���₷���͈̔͂ɗ}����j
        float wave = sinf(GetNowCount() * 0.01f) * 0.8f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, bar_alpha);

        // �w�i
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + bar_width, bar_y + bar_height + (int)wave,
            GetColor(40, 40, 40), TRUE);

        // �{�́i�Ԍn�F�����ۉ߂��Ȃ��悤�����j
        int bar_color = GetColor(230, 70, 70);
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + (int)(bar_width * hp_ratio), bar_y + bar_height + (int)wave,
            bar_color, TRUE);

        // �g
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + bar_width, bar_y + bar_height + (int)wave,
            GetColor(180, 180, 180), FALSE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Enemy4::Finalize() {
    // ���ɂȂ�
}
