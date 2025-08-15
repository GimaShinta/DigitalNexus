#include "Enemy4.h"
#include "../../../Utility/ProjectConfig.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"

Enemy4::Enemy4() {}
Enemy4::~Enemy4() {}

void Enemy4::Initialize()
{
    EnemyBase::Initialize();

    // ��{�p�����[�^�iEnemy3 Zako3�����j
    z_layer = 3;
    box_size = 12;
    hp = 10;

    collision.is_blocking = false;           // �o�ꒆ��OFF
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    // �摜�i�K�v�Ȃ獷���ւ�OK�j
    // ��: Enemy3��Zako3�Ɠ��X�v���C�g�ɍ��킹��
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    zako3_images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy41.png", 4, 4, 1, 104, 80);
    if (!zako3_images.empty()) zako3_image = zako3_images[0];

    // ������
    scale_min = 0.3f;
    scale_max = 0.8f;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;

    // �ʒu�i�f�t�H���g�E���SetAppearParams�ŏ㏑���j
    start_pos = Vector2D(D_WIN_MAX_X / 2, -100.0f);
    target_pos = Vector2D(D_WIN_MAX_X / 2, 220.0f);

    appear_duration = 1.2f;
    appear_timer = 0.0f;
    float_timer = 0.0f;
    state = Enemy4State::Appearing;
    mode = Enemy4Mode::Zako3;
}

void Enemy4::SetAppearParams(const Vector2D& s, const Vector2D& e, float time, bool from_left)
{
    start_pos = s;
    target_pos = e;
    appear_duration = (time > 0.05f) ? time : 0.05f;
    is_from_left = from_left;

    location = start_pos;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;
    state = Enemy4State::Appearing;
    appear_timer = 0.0f;
    float_timer = 0.0f;
}

void Enemy4::ShootToPlayer(float speed)
{
    GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
    auto* shot = gm->CreateObject<EnemyBullet1>(location);
    if (shot && player) {
        Vector2D dir = player->GetLocation() - location;
        dir.Normalize();
        shot->SetVelocity(dir * speed);
    }
}

void Enemy4::Update(float dt)
{
    float_timer += dt;

    switch (state)
    {
    case Enemy4State::Appearing: {
        appear_timer += dt;
        const float t = my_min(appear_timer / appear_duration, 1.0f);
        // easeOutCubic
        const float ease = 1.0f - powf(1.0f - t, 3.0f);

        // �ʒu��ԁ{�g��O�ɂ���o���h�c�h�ꉉ�o�iEnemy3 Zako3�����j
        location = start_pos + (target_pos - start_pos) * ease;
        location.y += 150.0f * sinf(t * DX_PI);

        // �����ڕ��
        alpha = static_cast<int>(255.0f * ease);
        if (alpha > 255) alpha = 255;

        // �X�P�[���͑傩�珬�ցi5.0 �� 0.8�j
        scale = 5.0f - 4.2f * ease;
        if (scale < scale_max) scale = scale_max;

        if (t >= 1.0f) {
            // ���B���Ɉꔭ��������
            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
            ShootToPlayer(0.003f);

            state = Enemy4State::Floating;
            base_loc = location;
            float_timer = 0.0f;
            alpha = 255;
            scale = scale_max;
            rotation = 0.0f;
        }
        break;
    }

    case Enemy4State::Floating: {
        // �������h���i���E���㉺�j
        location.x = base_loc.x + sinf(float_timer * 1.5f) * 10.0f;
        location.y = base_loc.y + sinf(float_timer * 2.0f) * 5.0f;

        if (float_timer >= 3.0f) {
            state = Enemy4State::Leaving;
            float_timer = 0.0f;
        }
        break;
    }

    case Enemy4State::Leaving: {
        // ��֑ޏ�i��ʊO�ŏ��Łj
        location.y -= dt * 300.0f;
        if (location.y + box_size.y < 0.0f) {
            is_destroy = true;
        }
        break;
    }
    }

    // �����蔻��i�o�ꒆOFF�^�ȍ~ON�j
    if (state == Enemy4State::Floating || state == Enemy4State::Leaving) {
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };
    }
    else {
        collision.is_blocking = false;
        collision.object_type = eObjectType::eNone;
        collision.hit_object_type.clear();
    }

    EnemyBase::Update(dt);
}

void Enemy4::Draw(const Vector2D& screen_offset) const
{
    const int img = (zako3_image != -1) ? zako3_image : -1;
    if (img == -1) return;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawRotaGraphF(location.x - screen_offset.x,
        location.y - screen_offset.y,
        scale, rotation, img, TRUE, zako3_flip);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

void Enemy4::Finalize()
{
    // �K�v�Ȃ��Еt��
}
