#include "Enemy6.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/ProjectConfig.h"

Enemy6::Enemy6() {}
Enemy6::Enemy6(const Vector2D& pos) { location = pos; }
Enemy6::~Enemy6() {}

void Enemy6::Initialize() {
    EnemyBase::Initialize();
    z_layer = 3;
    box_size = 12;
    hp = 12;

    // 出現中は無敵→落下開始でON（Enemy1/2流儀）
    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    auto* rm = Singleton<ResourceManager>::GetInstance();
    // 手持ちの軽めスプライトでOK（パスは環境に合わせて）
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_b.png", 12, 12, 1, 24, 24);
    if (!images.empty()) { image = images[0]; }

    appear_timer = 0.0f;
    scale = scale_min;
    alpha = 0;
    state = Enemy6State::Appearing;
}

void Enemy6::SetAppearParams(const Vector2D& start, float appear_time, float fall_speed) {
    start_pos = start;
    location = start_pos;
    appear_duration = (appear_time > 0.05f) ? appear_time : 1.0f;
    appear_timer = 0.0f;
    vy = (fall_speed > 0.0f) ? fall_speed : 100.0f;
    scale = scale_min;
    alpha = 0;
    state = Enemy6State::Appearing;
}

void Enemy6::Update(float dt) {
    switch (state) {
    case Enemy6State::Appearing: {
        appear_timer += dt;
        float t = appear_timer / appear_duration;
        if (t > 1.0f) t = 1.0f;
        float ease = t * t * (3.0f - 2.0f * t); // smoothstep

        scale = scale_min + (scale_max - scale_min) * ease;
        alpha = (int)(255 * ease);

        if (t >= 1.0f) {
            Shot(0.001f);
            state = Enemy6State::Falling;
            collision.is_blocking = true;
            collision.object_type = eObjectType::eEnemy;
            collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };
        }
        break;
    }

    case Enemy6State::Falling: {
        
        location.y += vy * dt;
        if (location.y - box_size.y > D_WIN_MAX_Y) state = Enemy6State::Leaving;
        break;
    }

    case Enemy6State::Leaving: {
        is_destroy = true;
        break;
    }
    }

    // 破壊処理
    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        auto* sm = Singleton<SEManager>::GetInstance();
        auto* em = Singleton<EffectManager>::GetInstance();
        sm->PlaySE(SE_NAME::Destroy);
        sm->PlaySE(SE_NAME::Dead1);
        Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
        int anim_id = em->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        em->SetScale(anim_id, 0.5f);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
    }

    // アニメ
    if (!images.empty()) {
        animation_time += dt;
        if (animation_time >= 0.1f) {
            animation_time -= 0.1f;
            animation_index = (animation_index + 1) % (int)images.size();
            image = images[animation_index];
        }
    }

    EnemyBase::Update(dt);
}

void Enemy6::Draw(const Vector2D& screen_offset) const {
    
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawRotaGraph(
        (int)(location.x - screen_offset.x),
        (int)(location.y - screen_offset.y),
        scale, 0.0f, image, TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
void Enemy6::Shot(float speed)
{
    // 到達時に1発だけ撃つ（Enemy3/Zako3相当）
    SEManager* am = Singleton<SEManager>::GetInstance();

    GameObjectManager* gm = Singleton<GameObjectManager>::GetInstance();
    EnemyBullet1* shot = gm->CreateObject<EnemyBullet1>(location);
    shot->SetPlayer(player);

    am->PlaySE(SE_NAME::Bullet01);
    am->ChangeSEVolume(SE_NAME::Bullet01, 100);

    if (shot && player)
    {
        Vector2D dir = player->GetLocation() - location;
        dir.Normalize();
        shot->SetVelocity(dir * speed);
    }

}

void Enemy6::Finalize() {}
