#include "Enemy5.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/ProjectConfig.h"

Enemy5::Enemy5() {}
Enemy5::Enemy5(const Vector2D& pos) { location = pos; }
Enemy5::~Enemy5() {}

void Enemy5::Initialize() {
    EnemyBase::Initialize();

    z_layer = 3;
    box_size = 12;
    hp = 10;

    // 出現中は無敵（Appearing）→進入完了後ON（Enemy2と同様の流儀） :contentReference[oaicite:3]{index=3}
    collision.is_blocking = false;
    collision.object_type = eObjectType::eNone;
    collision.hit_object_type.clear();

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    // 既存敵に合わせた取得（Enemy1参照のパス使用例） :contentReference[oaicite:4]{index=4}
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako1/anime_enemy30_a.png", 4, 4, 1, 32, 32);
    if (!images.empty()) {
        image = images[0];
        animation_time = 0.0f;
        animation_index = 0;
    }

    state = Enemy5State::Appearing;
    appear_timer = 0.0f;
    scale = scale_min;
    alpha = 0;
}

void Enemy5::SetAppearParams(const Vector2D& start, float appear_time) {
    start_pos = start;
    location = start_pos;
    appear_duration = (appear_time > 0.05f) ? appear_time : 1.0f;
    appear_timer = 0.0f;
    scale = scale_min;
    alpha = 0;
    state = Enemy5State::Appearing;
}

void Enemy5::SetFormationParams(int idx, int total, bool from_left,
    float spacing, float speed,
    int col_idx, int col_total_, float col_stride_) {
    line_index = (idx < 0) ? 0 : idx;
    line_total = (total <= 0) ? 1 : total;
    from_left_up = from_left;
    spacing_px = spacing;
    move_speed = speed;

    column_index = (col_idx < 0) ? 0 : col_idx;
    column_total = (col_total_ <= 0) ? 1 : col_total_;
    column_stride = col_stride_;

    // 進行方向
    Vector2D dir = from_left_up ? Vector2D(+1, +1) : Vector2D(-1, +1);
    dir.Normalize();

    // 直角方向（重なり回避用）
    Vector2D perp(-dir.y, dir.x); // 左法線
    perp.Normalize();

    velocity = dir * move_speed;

    // 斜めオフセット（縦隊の距離）
    Vector2D offset_line = dir * (spacing_px * line_index);

    // 列オフセット：中央基準で左右に並べる
    float col_centered = (float)column_index - (float)(column_total - 1) * 0.5f;
    Vector2D offset_col = perp * (column_stride * col_centered);

    location = start_pos + offset_line + offset_col;
}


void Enemy5::Update(float dt) {
    switch (state) {
    case Enemy5State::Appearing: {
        appear_timer += dt;
        float t = appear_timer / appear_duration;
        if (t > 1.0f) t = 1.0f;
        // smoothstep（Enemy1/2の出現と同じノリ） :contentReference[oaicite:5]{index=5} :contentReference[oaicite:6]{index=6}
        float ease = t * t * (3.0f - 2.0f * t);

        scale = scale_min + (scale_max - scale_min) * ease;
        alpha = static_cast<int>(255 * ease);

        if (t >= 1.0f) {
            state = Enemy5State::Moving;
            collision.is_blocking = true;
            collision.object_type = eObjectType::eEnemy;
            collision.hit_object_type = { eObjectType::eAttackShot, eObjectType::eBeam };
        }
        break;
    }

    case Enemy5State::Moving: {
        location += velocity * dt;

        // 画面外に出たら終了
        if (location.x < -80.0f || location.x > D_WIN_MAX_X + 80.0f ||
            location.y > D_WIN_MAX_Y + 80.0f) {
            state = Enemy5State::Leaving;
        }
        break;
    }

    case Enemy5State::Leaving: {
        is_destroy = true;
        break;
    }
    }


    // 被破壊処理（Enemy1/2 と同じ雰囲気） :contentReference[oaicite:7]{index=7} :contentReference[oaicite:8]{index=8}
    if (hp <= 0) {
        is_destroy = true;
        DropItems();
        auto sm = Singleton<SEManager>::GetInstance();
        auto em = Singleton<EffectManager>::GetInstance();
        sm->PlaySE(SE_NAME::Destroy);
        sm->PlaySE(SE_NAME::Dead1);
        Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
        int anim_id = em->PlayerAnimation(EffectName::eExprotion2, location, 0.035f, false);
        em->SetScale(anim_id, 0.5f);
        Singleton<ScoreData>::GetInstance()->AddScore(500);
    }

    // アニメ更新（Enemy2準拠） :contentReference[oaicite:9]{index=9}
    if (!images.empty()) {
        animation_time += dt;
        if (animation_time >= 0.1f) {
            animation_time -= 0.1f;
            animation_index = (animation_index + 1) % static_cast<int>(images.size());
            image = images[animation_index];
        }
    }

    EnemyBase::Update(dt);
}

void Enemy5::Draw(const Vector2D& screen_offset) const {
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawRotaGraph(
        static_cast<int>(location.x - screen_offset.x),
        static_cast<int>(location.y - screen_offset.y),
        scale, 0.0f, image, TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Enemy5::Finalize() {}
