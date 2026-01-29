#include "Enemy8.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/ResourceManager.h"

void Enemy8::Initialize() {
    EnemyBase::Initialize();

    hp = 8;                 // Enemy1 と同程度
    z_layer = 2;
    box_size = Vector2D(12, 12);

    // 画像：Enemy1と同じアセット（必要なら後で差し替え）
    auto* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako2/anime_enemy34.png", 8, 8, 1, 56, 56);
    image_index = 0;
    image = images.empty() ? -1 : images[0];

    // 登場中は無敵
    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    appear_timer = 0.0f;
    alpha = 0;
    scale = scale_min;

    shot_fired = false;
}

void Enemy8::Finalize() {
    EnemyBase::Finalize();
}

void Enemy8::SetAppearParams(const Vector2D& start, const Vector2D& end, float time) {
    start_location = start;
    target_location = end;
    appear_duration = time;
    location = start_location;
    appear_timer = 0.0f;
    scale = scale_min;
    alpha = 0;
    state = Enemy8State::Appearing;
}

// Enemy8.cpp
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"  // ←弾クラスはこれでOK（今のまま）

void Enemy8::ShootOnce() {
    if (shot_fired) return;
    shot_fired = true;

    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();

    Vector2D dir(0.0f, 1.0f);           // デフォは下向き
    if (player) {
        dir = player->GetLocation() - location;
        // ★ LengthSq() を使わずに長さ?を自前計算
        float len2 = dir.x * dir.x + dir.y * dir.y;
        if (len2 > 1e-6f) {
            dir.Normalize();            // 0除算ガードの上で正規化
        }
        else {
            dir = Vector2D(0.0f, 1.0f); // 同一点なら下向き固定
        }
    }

    const float spd = 160.0f;           // EnemyBullet2 に合わせた見やすい速度
    if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
        b->SetVelocity(dir * spd);
        b->SetPlayer(player);
    }

    se->PlaySE(SE_NAME::EnemyShot);
}



void Enemy8::Update(float dt) {
    appear_timer += dt;

    if (state == Enemy8State::Appearing) {
        float t = appear_timer / appear_duration;
        if (t > 1.0f) t = 1.0f;
        // smoothstep
        float ease = t * t * (3.0f - 2.0f * t);

        Vector2D prev = location;
        location = start_location + (target_location - start_location) * ease;
        velocity = (location - prev) / dt;

        scale = scale_min + (scale_max - scale_min) * ease;
        alpha = static_cast<int>(255.0f * ease);

        if (t >= 1.0f) {
            // 当たり判定 ON
            collision.is_blocking = true;
            collision.object_type = eObjectType::eEnemy;
            collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };

            // 一発だけ撃つ
            Shot(0.002f);

            // ゆっくり下へ去る
            velocity.x = 0.0f;
            velocity.y = leave_vy;
            state = Enemy8State::Leaving;
        }
    }
    else { // Leaving
        location += velocity * dt;

        if (location.y > D_WIN_MAX_Y + box_size.y) {
            is_destroy = true;
        }
    }

    // 被弾処理
    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        SEManager::GetInstance()->PlaySE(SE_NAME::Destroy);
        SEManager::GetInstance()->PlaySE(SE_NAME::Dead1);
        Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 50);
        Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Destroy, 80);
        int anim_id = EffectManager::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        Singleton<EffectManager>::GetInstance()->SetScale(anim_id, 0.5f);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
    }

    // 簡単アニメ
    if (!images.empty()) {
        animation_time += dt;
        if (animation_time >= 0.1f) {
            animation_time = 0.0f;
            image_index = (image_index + 1) % (int)images.size();
            image = images[image_index];
        }
    }

    EnemyBase::Update(dt);
}

void Enemy8::Draw(const Vector2D& screen_offset) const {


    if (image < 0) return;
    int a = alpha; if (a < 0) a = 0; if (a > 255) a = 255;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawRotaGraph(
        (int)(location.x - screen_offset.x),
        (int)(location.y - screen_offset.y),
        scale,
        0.0f,
        image,
        TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy8::Shot(float speed)
{
    // 到達時に1発だけ撃つ（Enemy3/Zako3相当）
    SEManager* am = Singleton<SEManager>::GetInstance();

    GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
    EnemyBullet1* shot = gm->CreateObject<EnemyBullet1>(location);
    shot->SetPlayer(player);

    am->PlaySE(SE_NAME::EnemyShot);
    am->ChangeSEVolume(SE_NAME::EnemyShot, 90);

    if (shot && player)
    {
        Vector2D dir = player->GetLocation() - location;
        dir.Normalize();
        shot->SetVelocity(dir * speed);
    }

}