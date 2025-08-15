#include "Enemy2.h"
#include "../../../Utility/ProjectConfig.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"

Enemy2::Enemy2() {}
Enemy2::~Enemy2() {}

void Enemy2::Initialize()
{
    EnemyBase::Initialize();

    z_layer = 3;
    box_size = 12;
    hp = 10;

    // �o�����͓����蔻��OFF�iFloating/Leaving��ON�ɂ���j �� Enemy3����
    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    // Enemy3 Zako3 �Ɠ��n���̉摜
    zako3_images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy41.png", 4, 4, 1, 104, 80);
    if (!zako3_images.empty()) zako3_image = zako3_images[0];

    // �����Zako3���C�N�i������ʁj
    scale_min = 0.3f;
    scale_max = 0.8f;
    appear_duration = 1.2f;

    start_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y + 50);
    target_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2 - 50);

    location = start_pos;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;

    // ����͑��o���J�n�iLineRise��Waiting�ő҂j
    state = (mode == Enemy2Mode::LineRise) ? Enemy2State::Waiting : Enemy2State::Appearing;
    float_timer = 0.0f;
    appear_timer = 0.0f;
    wait_timer = 0.0f;
}

void Enemy2::SetLineParams(int index, int total, float interval_sec)
{
    line_index = (index < 0) ? 0 : index;
    line_total = (total <= 0) ? 1 : total;
    line_interval = (interval_sec < 0.0f) ? 0.0f : interval_sec;

    // �����̑ҋ@���ԁi���ԁ~�Ԋu�j
    wait_time = line_index * line_interval;

    // ���[�h��LineRise�Ȃ�Waiting����J�n������
    if (mode == Enemy2Mode::LineRise) {
        state = Enemy2State::Waiting;
        alpha = 0;
        scale = scale_min;
        wait_timer = 0.0f;
        appear_timer = 0.0f;
        location = start_pos; // �����őҋ@
    }
}

void Enemy2::SetAppearParams(const Vector2D& s, const Vector2D& e, float time, bool from_left)
{
    start_pos = s;
    target_pos = e;
    appear_duration = (time > 0.05f) ? time : 1.2f; // �Œ�l���ꉞ�m��
    is_from_left = from_left;

    location = start_pos;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;

    // LineRise��Waiting�A����ȊO��Appearing
    state = (mode == Enemy2Mode::LineRise) ? Enemy2State::Waiting : Enemy2State::Appearing;
    float_timer = 0.0f;
    appear_timer = 0.0f;
    wait_timer = 0.0f;
}

void Enemy2::ShootToPlayer(float speed)
{
    GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
    GameObjectBase* shot = gm->CreateObject<EnemyBullet1>(location);
    if (shot && player)
    {
        Vector2D dir = player->GetLocation() - location;
        dir.Normalize();
        shot->SetVelocity(dir * speed);
    }
}

void Enemy2::Shot(float /*delta_second*/)
{
    // ���B����1���������iEnemy3/Zako3�����j
    ShootToPlayer(0.003f);
}

void Enemy2::Update(float delta_second)
{
    float_timer += delta_second;

    switch (state)
    {
    case Enemy2State::Waiting:
    {
        // LineRise�F���Ԃ�����܂œ����őҋ@
        wait_timer += delta_second;
        alpha = 0;
        if (wait_timer >= wait_time) {
            // ��������o���A�j���J�n
            state = Enemy2State::Appearing;
            appear_timer = 0.0f;
            // �J�n���ʒu��start_pos�i�����z��j
            location = start_pos;
        }
        break;
    }

    case Enemy2State::Appearing:
    {
        // Zako3�����̏㉺�ʁi������Ȃ̂ŕ����̓}�C�i�X�j
        appear_timer += delta_second;
        float t = my_min(appear_timer / appear_duration, 1.0f);
        float ease_t = 1 - powf(1 - t, 3);

        location = start_pos + (target_pos - start_pos) * ease_t;
        location.y -= 150.0f * sinf(t * DX_PI);  // ����̌�

        alpha = static_cast<int>(255 * ease_t);
        scale = 5.0f - 4.2f * ease_t;

        if (t >= 1.0f) {
            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
            Shot(delta_second); // ���B���Ɉ�x����
            state = Enemy2State::Floating;
            base_location = location;
            float_timer = 0.0f;
            scale = scale_max;
            alpha = 255;
            rotation = 0.0f;
        }
        break;
    }

    case Enemy2State::Floating:
    {
        // �����݂ӂ�ӂ�
        location.x = base_location.x + sinf(float_timer * 1.5f) * 10.0f;
        location.y = base_location.y + sinf(float_timer * 2.0f) * 5.0f;

        if (float_timer >= 3.0f) {
            state = Enemy2State::Leaving;
            float_timer = 0.0f;
        }
        break;
    }

    case Enemy2State::Leaving:
    {
        // ������ɑޏ�
        location.y -= delta_second * 300.0f;
        if (location.y + box_size.y < 0) {
            is_destroy = true;
        }
        break;
    }

    default: break;
    }

    // �����蔻��FFloating/Leaving��ON�iEnemy3�����j
    if (state == Enemy2State::Floating || state == Enemy2State::Leaving) {
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };
    }
    else {
        collision.is_blocking = false;
        collision.object_type = eObjectType::eNone;
        collision.hit_object_type.clear();
    }

    // �j�󏈗��iEnemy3�Ƒ����݁j
    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        SEManager* sm = Singleton<SEManager>::GetInstance();
        EffectManager* manager = Singleton<EffectManager>::GetInstance();
        sm->PlaySE(SE_NAME::Destroy);
        int anim_id = manager->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        manager->SetScale(anim_id, 0.5f);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
        Singleton<ShakeManager>::GetInstance()->StartShake(0.5, 3, 3);
    }

    EnemyBase::Update(delta_second);
}

void Enemy2::Draw(const Vector2D& screen_offset) const
{
    const int img = (zako3_image != -1) ? zako3_image : -1;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawRotaGraphF(
        location.x - screen_offset.x,
        location.y - screen_offset.y,
        scale, rotation, img, TRUE, zako3_flip
    );
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

void Enemy2::Finalize() {}
