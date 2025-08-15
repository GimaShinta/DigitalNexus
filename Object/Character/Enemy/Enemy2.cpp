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

    // 出現中は当たり判定OFF（Floating/LeavingでONにする） ← Enemy3準拠
    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    // Enemy3 Zako3 と同系統の画像
    zako3_images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy41.png", 4, 4, 1, 104, 80);
    if (!zako3_images.empty()) zako3_image = zako3_images[0];

    // 既定はZako3ライク（下から弧）
    scale_min = 0.3f;
    scale_max = 0.8f;
    appear_duration = 1.2f;

    start_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y + 50);
    target_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2 - 50);

    location = start_pos;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;

    // 既定は即出現開始（LineRiseはWaitingで待つ）
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

    // 自分の待機時間（順番×間隔）
    wait_time = line_index * line_interval;

    // モードがLineRiseならWaitingから開始させる
    if (mode == Enemy2Mode::LineRise) {
        state = Enemy2State::Waiting;
        alpha = 0;
        scale = scale_min;
        wait_timer = 0.0f;
        appear_timer = 0.0f;
        location = start_pos; // 透明で待機
    }
}

void Enemy2::SetAppearParams(const Vector2D& s, const Vector2D& e, float time, bool from_left)
{
    start_pos = s;
    target_pos = e;
    appear_duration = (time > 0.05f) ? time : 1.2f; // 最低値を一応確保
    is_from_left = from_left;

    location = start_pos;
    scale = scale_min;
    alpha = 0;
    rotation = 0.0f;

    // LineRiseはWaiting、それ以外はAppearing
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
    // 到達時に1発だけ撃つ（Enemy3/Zako3相当）
    ShootToPlayer(0.003f);
}

void Enemy2::Update(float delta_second)
{
    float_timer += delta_second;

    switch (state)
    {
    case Enemy2State::Waiting:
    {
        // LineRise：順番が来るまで透明で待機
        wait_timer += delta_second;
        alpha = 0;
        if (wait_timer >= wait_time) {
            // ここから出現アニメ開始
            state = Enemy2State::Appearing;
            appear_timer = 0.0f;
            // 開始時位置はstart_pos（下側想定）
            location = start_pos;
        }
        break;
    }

    case Enemy2State::Appearing:
    {
        // Zako3相当の上下弧（下からなので符号はマイナス）
        appear_timer += delta_second;
        float t = my_min(appear_timer / appear_duration, 1.0f);
        float ease_t = 1 - powf(1 - t, 3);

        location = start_pos + (target_pos - start_pos) * ease_t;
        location.y -= 150.0f * sinf(t * DX_PI);  // 浮上の弧

        alpha = static_cast<int>(255 * ease_t);
        scale = 5.0f - 4.2f * ease_t;

        if (t >= 1.0f) {
            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
            Shot(delta_second); // 到達時に一度撃つ
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
        // 小刻みふわふわ
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
        // 上方向に退場
        location.y -= delta_second * 300.0f;
        if (location.y + box_size.y < 0) {
            is_destroy = true;
        }
        break;
    }

    default: break;
    }

    // 当たり判定：Floating/LeavingでON（Enemy3準拠）
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

    // 破壊処理（Enemy3と足並み）
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
