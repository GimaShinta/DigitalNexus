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

    // ---- 基本パラメータ ----
    hp = 8000;
    is_destroy = false;
    is_alive = true;
    box_size = Vector2D(50, 50);
    z_layer = 2;

    scale_from = 4.0f;
    scale_to = 3.0f;
    scale_draw = scale_from;

    // 画像
    auto* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy75_a.png", 6, 6, 1, 40, 40);
    image_index = 0;
    image = images.empty() ? -1 : images[0];

    // 落下で登場（手前から）
    enter_start = Vector2D(D_WIN_MAX_X * 0.5f, -240.0f);
    target_pos = Vector2D(D_WIN_MAX_X * 0.5f, D_WIN_MAX_Y * 0.32f);
    location = enter_start;
    alpha = 0;

    // 登場中は無敵（Battle移行で有効化）
    collision.is_blocking = false;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.clear();

    pattern = Enemy4Pattern::Entrance;
    pattern_timer = 0.0f;

    // 徘徊/攻撃
    wander_timer = 0.0f;
    next_wander = 1.2f;
    noise_t = 0.0f;
    shot_timer = 0.0f;
    attack_mode = 0;

    // フラッシュ
    shot_flash_t = 0.0f;
}

void Enemy4::Update(float dt) {
    pattern_timer += dt;

    switch (pattern) {
    case Enemy4Pattern::Entrance: UpdateEntrance(dt); break;
    case Enemy4Pattern::Battle:   UpdateBattle(dt);   break;
    case Enemy4Pattern::Retreat:  UpdateRetreat(dt);  break;
    case Enemy4Pattern::Dead:     UpdateDead(dt);     break;
    }

    // 画像アニメ
    if (!images.empty()) {
        anim_time += dt;
        if (anim_time >= 0.09f) {
            anim_time -= 0.09f;
            image_index = (image_index + 1) % (int)images.size();
            image = images[image_index];
        }
    }

    // 発射フラッシュ減衰
    if (shot_flash_t > 0.0f) {
        shot_flash_t -= dt;
        if (shot_flash_t < 0.0f) shot_flash_t = 0.0f;
    }

    on_hit = false;
    EnemyBase::Update(dt);
}

void Enemy4::UpdateEntrance(float dt) {
    float t = Saturate(pattern_timer / enter_time);
    float e = EaseOutCubic(t);

    location.y = enter_start.y + (target_pos.y - enter_start.y) * e;
    scale_draw = scale_from + (scale_to - scale_from) * e;

    alpha = (int)(255.0f * e);
    if (alpha > 255) alpha = 255;

    if (t >= 1.0f) {
        // 当たり判定有効化
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type.clear();
        collision.hit_object_type.push_back(eObjectType::eAttackShot);
        collision.hit_object_type.push_back(eObjectType::eBeam);

        pattern = Enemy4Pattern::Battle;
        pattern_timer = 0.0f;
        shot_timer = 0.0f;

        // 初期徘徊ターゲット
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(200) - 100),
            D_WIN_MAX_Y * 0.28f + (GetRand(140) - 70)
        );
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f);
    }
}

void Enemy4::UpdateBattle(float dt) {
    // 徘徊
    wander_timer += dt;
    noise_t += dt;

    if (wander_timer >= next_wander) {
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f);
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(280) - 140),
            D_WIN_MAX_Y * 0.28f + (GetRand(180) - 90)
        );
    }

    Vector2D to = wander_target - location;
    location += to * (2.8f * dt);

    // 微ノイズ
    float nX = sinf(noise_t * 2.3f) * 6.0f + sinf(noise_t * 1.7f + DX_PI / 3) * 4.0f;
    float nY = cosf(noise_t * 2.0f) * 5.0f + sinf(noise_t * 2.6f + DX_PI / 5) * 3.0f;
    location.x += nX * dt * 4.0f;
    location.y += nY * dt * 4.0f;

    // ---- ここだけ置き換え（UpdateBattle 内の該当部分）----
    shot_timer += dt;

    // 短めの間隔に調整（HPに応じてやや可変）
    // hp_ratio は 0.0～1.0 にクランプしてから使用（上限1.0）
    float interval;
    if (easy_mode) {
        interval = 0.90f;  // Easy でも少し短め
    }
    else {
        float hp_ratio = (float)hp / 8000.0f;
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;
        if (hp_ratio > 1.0f) hp_ratio = 1.0f;
        interval = 0.70f * hp_ratio + 0.24f; // 0.24～0.94 秒程度
    }

    if (shot_timer >= interval) {
        shot_timer = 0.0f;

        if (CanShoot()) {
            if (easy_mode) {
                Shot4WayAim();
            }
            else {
                switch (attack_mode % 3) {
                case 0: Shot4WayAim();             break;
                case 1: Shot4WayWide();            break;
                default: Shot4WayStaggeredBurst(); break;
                }
                attack_mode++;
            }
            // 発射フラッシュ
            shot_flash_t = 0.12f;
        }
    }


    if (hp <= 0) {
        pattern = Enemy4Pattern::Dead;
        pattern_timer = 0.0f;
    }
}

void Enemy4::ForceRetreat() {
    if (pattern == Enemy4Pattern::Dead || pattern == Enemy4Pattern::Retreat) return;
    collision.is_blocking = false;
    collision.hit_object_type.clear();
    pattern = Enemy4Pattern::Retreat;
    pattern_timer = 0.0f;
}

void Enemy4::UpdateRetreat(float dt) {
    float up_speed = 260.0f + 120.0f * (pattern_timer);
    location.y -= up_speed * dt;

    int a = 255 - (int)(pattern_timer * (255.0f / 1.2f));
    if (a < 0) a = 0;
    alpha = a;

    if (location.y < -120.0f || alpha == 0) {
        is_destroy = true;
    }
}

void Enemy4::UpdateDead(float) {
    Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.06f, false);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Destroy);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Dead1);
    Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
    Singleton<ScoreData>::GetInstance()->AddScore(1500);
    is_destroy = true;
}

// ===== 4Way 系のみ =====

void Enemy4::Shot4WayAim() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();
    if (!player) return;

    // 狙い基準の4Way（±13°相当）
    Vector2D toP = player->GetLocation() - location;
    toP.Normalize();
    float base = atan2f(toP.y, toP.x);

    const int n = 4;
    const float half = Deg2Rad(13.0f); // spread 半分
    const float speed = 180.0f;

    for (int i = 0; i < n; ++i) {
        // -half ～ +half を n-1 分割
        float ang = base + (-half + (half * 2.0f) * i / (n - 1));
        Vector2D dir(cosf(ang), sinf(ang));
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dir * speed);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
    se->ChangeSEVolume(SE_NAME::EnemyShot, 90);
}

void Enemy4::Shot4WayWide() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();
    if (!player) return;

    // ちょい広めの4Way（±18°）
    Vector2D toP = player->GetLocation() - location;
    toP.Normalize();
    float base = atan2f(toP.y, toP.x);

    const int n = 4;
    const float half = Deg2Rad(18.0f);
    const float speed = 170.0f; // やや遅め＝見やすい

    for (int i = 0; i < n; ++i) {
        float ang = base + (-half + (half * 2.0f) * i / (n - 1));
        Vector2D dir(cosf(ang), sinf(ang));
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dir * speed);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::Shot4WayStaggeredBurst() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();
    if (!player) return;

    // 毎回少し左右に寄せる（±8°のベースに ±6°のシフト）
    static bool flip = false;
    flip = !flip;

    Vector2D toP = player->GetLocation() - location;
    toP.Normalize();
    float base = atan2f(toP.y, toP.x);

    const int n = 4;
    const float half = Deg2Rad(8.0f);
    const float shift = Deg2Rad(flip ? 6.0f : -6.0f);
    const float speed = 190.0f; // やや速め

    for (int i = 0; i < n; ++i) {
        float ang = base + shift + (-half + (half * 2.0f) * i / (n - 1));
        Vector2D dir(cosf(ang), sinf(ang));
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dir * speed);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
}



bool Enemy4::CanShoot() const {
    // 充分に画面内かつフェードイン済み（透明に近い状態では撃たない＝“見えない弾”を防止）
    const int margin = 8;
    bool on_screen =
        location.x >= margin && location.x <= (D_WIN_MAX_X - margin) &&
        location.y >= margin && location.y <= (D_WIN_MAX_Y - margin);

    return (pattern == Enemy4Pattern::Battle) && on_screen && (alpha >= 160);
}

void Enemy4::OnHitCollision(GameObjectBase* hit_object) {
    auto type = hit_object->GetCollision().object_type;

    if (type == eObjectType::eAttackShot) {
        hp -= 10;
    }
    if (type == eObjectType::eBeam) {
        // ビームは一定間隔で削る（過剰多段の見えないダメージ抑制）
        beam_damage_timer += 1.0f / 60.0f;
        if (beam_damage_timer >= 0.35f) {
            hp -= 10;
            beam_damage_timer = 0.0f;
        }
    }
}

void Enemy4::Draw(const Vector2D& screen_offset) const {
    if (image < 0) return;

    // 登場の落下ストリーク
    if (pattern == Enemy4Pattern::Entrance) {
        float t = Saturate(pattern_timer / enter_time);
        int streaks = 8;
        int a = (int)(120.0f * (1.0f - t));
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

    // 本体
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawRotaGraph(
        (int)(location.x - screen_offset.x),
        (int)(location.y - screen_offset.y),
        scale_draw,
        0.0f,
        image,
        TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 発射フラッシュ（“今、弾が出た”を目で追える）
    if (shot_flash_t > 0.0f) {
        int a = (int)(220.0f * (shot_flash_t / 0.12f));
        SetDrawBlendMode(DX_BLENDMODE_ADD, a);
        const int rays = 10;
        const float r0 = 6.0f;
        const float r1 = 22.0f;
        for (int i = 0; i < rays; ++i) {
            float ang = (DX_PI * 2.0f) * (i / (float)rays);
            int x0 = (int)(location.x - screen_offset.x + std::cos(ang) * r0);
            int y0 = (int)(location.y - screen_offset.y + std::sin(ang) * r0);
            int x1 = (int)(location.x - screen_offset.x + std::cos(ang) * r1);
            int y1 = (int)(location.y - screen_offset.y + std::sin(ang) * r1);
            DrawLine(x0, y0, x1, y1, GetColor(180, 230, 255));
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // HPゲージ
    //{
    //    float hp_ratio = (float)hp / 8000.0f;   // 元: 8000.0f
    //    if (hp_ratio < 0.0f) hp_ratio = 0.0f;
    //    if (hp_ratio > 1.0f) hp_ratio = 1.0f;

    //    float fade_ratio = (pattern_timer < 0.0f) ? 0.0f
    //        : ((pattern_timer > 1.2f) ? 1.0f : (pattern_timer / 1.2f));
    //    int bar_alpha = (int)(fade_ratio * 200);

    //    const int bar_width = 140;
    //    const int bar_height = 5;
    //    const int bar_x = (int)(location.x - screen_offset.x) - bar_width / 2;
    //    const int bar_y = (int)(location.y - screen_offset.y) - (int)(scale_draw * 24.0f) - 8;

    //    float wave = sinf(GetNowCount() * 0.01f) * 0.8f;

    //    SetDrawBlendMode(DX_BLENDMODE_ALPHA, bar_alpha);

    //    DrawBox(bar_x, bar_y + (int)wave,
    //        bar_x + bar_width, bar_y + bar_height + (int)wave,
    //        GetColor(40, 40, 40), TRUE);

    //    int bar_color = GetColor(230, 70, 70);
    //    DrawBox(bar_x, bar_y + (int)wave,
    //        bar_x + (int)(bar_width * hp_ratio), bar_y + bar_height + (int)wave,
    //        bar_color, TRUE);

    //    DrawBox(bar_x, bar_y + (int)wave,
    //        bar_x + bar_width, bar_y + bar_height + (int)wave,
    //        GetColor(180, 180, 180), FALSE);

    //    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    //}

}

void Enemy4::SetEasyMode(bool on) {
    easy_mode = on;
    if (easy_mode && hp > 600) hp = 600; // EasyはHP圧縮
}

bool Enemy4::IsCleared() const {
    return (pattern == Enemy4Pattern::Dead) || is_destroy;
}

void Enemy4::Finalize() {
    // nothing
}
