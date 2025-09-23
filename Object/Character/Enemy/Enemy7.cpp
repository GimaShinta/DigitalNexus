#include "Enemy7.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Object/GameObjectManager.h"
#include <cmath>

Enemy7::Enemy7() {}
Enemy7::~Enemy7() {}

void Enemy7::Initialize() {
    EnemyBase::Initialize();

    // 基本パラメータ（Enemy4相当から微調整）
    hp = 4000;                          // 少し硬め
    is_destroy = false;
    z_layer = 2;


    box_size = Vector2D(72, 62);   // ← 68?80 でもOK。まずは 72 程度で当てやすく
    scale_from = 4.2f;               // 登場時の見た目サイズを大きめ
    scale_to = 3.1f;               // 戦闘中も一回り大きく
    scale_draw = scale_from;

    // 画像：まずは Enemy4 と同じアトラスを使用（後で差し替え可）
    auto* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako2/anime_enemy14.png", 5, 5, 1, 48, 48);
    image_index = 0;
    image = images.empty() ? -1 : images[0];

    // 登場位置（上から降りる）
    enter_start = Vector2D(D_WIN_MAX_X * 0.5f, -240.0f);
    target_pos = Vector2D(D_WIN_MAX_X * 0.5f, D_WIN_MAX_Y * 0.30f);
    location = enter_start;
    alpha = 0;

    // Entrance 中は無敵
    collision.is_blocking = false;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.clear();

    pattern = Enemy7Pattern::Entrance;
    pattern_timer = 0.0f;

    // 徘徊/攻撃
    wander_timer = 0.0f;
    next_wander = 1.0f;
    noise_t = 0.0f;
    shot_timer = 0.0f;
    attack_mode = 0;

    shot_flash_t = 0.0f;
}

void Enemy7::Finalize() {
    EnemyBase::Finalize();
}

void Enemy7::Update(float dt) {
    pattern_timer += dt;

    switch (pattern) {
    case Enemy7Pattern::Entrance: UpdateEntrance(dt); break;
    case Enemy7Pattern::Battle:   UpdateBattle(dt);   break;
    case Enemy7Pattern::Retreat:  UpdateRetreat(dt);  break;
    case Enemy7Pattern::Dead:     UpdateDead(dt);     break;
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

void Enemy7::Draw(const Vector2D& screen_offset) const {
    // 透明度（フェードイン/退場用）
    const int a = alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha);

   

    // ========== 本体スプライト ==========
    if (image >= 0) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

         if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
        // scale_draw をそのまま拡大率に使う
        DrawRotaGraph(
            (int)(location.x - screen_offset.x),
            (int)(location.y - screen_offset.y),
            scale_draw,                 // ←★ ここで大きさを制御
            0.0f,
            image,
            TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ========== 砲口フラッシュ（スケール連動） ==========
    if (shot_flash_t > 0.0f) {
        int fa = (int)(shot_flash_t * 220);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, fa);

        // 元の28にスケールを掛ける（微調整したければ係数を変えてOK）
        const int r = (int)(28.0f * scale_draw * 0.9f);

        DrawCircle(
            (int)(location.x - screen_offset.x),
            (int)(location.y - screen_offset.y),
            r,                          // ←★ 大きさが連動
            GetColor(180, 220, 255),
            FALSE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // （必要なら当たり判定デバッグ）
    // DrawBox(...) / DrawCircle(...) で box_size を可視化してもOK
}


void Enemy7::UpdateEntrance(float dt) {
    float t = Saturate(pattern_timer / enter_time);
    float e = EaseOutCubic(t);

    location.y = enter_start.y + (target_pos.y - enter_start.y) * e;
    scale_draw = scale_from + (scale_to - scale_from) * e;

    alpha = (int)(255.0f * e);
    if (alpha > 255) alpha = 255;

    if (t >= 1.0f) {
        // 当たり判定 ON
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type.clear();
        collision.hit_object_type.push_back(eObjectType::eAttackShot);
        collision.hit_object_type.push_back(eObjectType::eBeam);

        pattern = Enemy7Pattern::Battle;
        pattern_timer = 0.0f;
        shot_timer = 0.0f;

        // ★ アンカー（基準点）を「今の場所」に固定
        anchor_pos = location;

        // ★ アンカーが半画面範囲外に出ないよう軽くクランプ
        if (use_bounds) {
            if (anchor_pos.x < wander_bounds.xmin) anchor_pos.x = wander_bounds.xmin;
            if (anchor_pos.x > wander_bounds.xmax) anchor_pos.x = wander_bounds.xmax;
            if (anchor_pos.y < wander_bounds.ymin) anchor_pos.y = wander_bounds.ymin;
            if (anchor_pos.y > wander_bounds.ymax) anchor_pos.y = wander_bounds.ymax;
            location = anchor_pos; // 念のため同期
        }

        // 個体差（速度）を少し付与
        orbit_speed = 0.6f + (GetRand(40) / 100.0f); // 0.6～1.0
        orbit_t = (GetRand(628) / 100.0f);           // 0～約6.28のランダム位相

        // 従来のランダム徘徊は使わないので、タイマ類はリセットだけ
        wander_timer = 0.0f;
        next_wander = 1.0f;
        noise_t = 0.0f;
    }
}

void Enemy7::UpdateBattle(float dt) {
    // 軌道（アンカー中心の小さな楕円）
    orbit_t += dt * orbit_speed;

    // 目標位置：アンカー + 小さな楕円 + ごく小さな縦ゆらぎ
    Vector2D desired = anchor_pos + Vector2D(
        orbit_rx * cosf(orbit_t),
        orbit_ry * sinf(orbit_t * 1.25f)
    );
    desired.y += bob_amp_y * sinf(orbit_t * bob_spd);

    // 半画面の範囲を超えないよう軽くクランプ（見切れ対策）
    if (use_bounds) {
        if (desired.x < wander_bounds.xmin) desired.x = wander_bounds.xmin;
        if (desired.x > wander_bounds.xmax) desired.x = wander_bounds.xmax;
        if (desired.y < wander_bounds.ymin) desired.y = wander_bounds.ymin;
        if (desired.y > wander_bounds.ymax) desired.y = wander_bounds.ymax;
    }

    // スムーズ追従（大きく動かないよう緩め）
    Vector2D to = desired - location;
    location += to * (1.6f * dt);  // ← 以前の 2.6f より遅く

    // 微ノイズは控えめ（オフでもOK）
    noise_t += dt;
    location.x += sinf(noise_t * 1.1f) * 0.5f * dt;
    location.y += cosf(noise_t * 1.3f) * 0.4f * dt;

    // 攻撃（あなたの2種ローテ仕様）
    shot_timer += dt;
    float interval = easy_mode ? 2.0f : 1.5f;

    if (shot_timer >= interval) {
        shot_timer = 0.0f;
        if (CanShoot()) {
            // 3回に1回はリング（= 4回に1回の頻度）
            if ((attack_mode % 4) == 3) {
                ShotRing12();   // ←中身は“6方向ゆっくり”にしてます（下の変更点②）
            }
            else {
                Shot6WayFan();  // ←中身は“3Wayゆっくり”にしてます（下の変更点②）
            }
            attack_mode++;
            shot_flash_t = 0.12f;
        }
    }

    if (hp <= 0) {
        pattern = Enemy7Pattern::Dead;
        pattern_timer = 0.0f;
    }
}



void Enemy7::UpdateRetreat(float dt) {
    float up_speed = 260.0f + 120.0f * (pattern_timer);
    location.y -= up_speed * dt;

    int a = 255 - (int)(pattern_timer * (255.0f / 1.2f));
    if (a < 0) a = 0;
    alpha = a;

    if (location.y < -120.0f || alpha == 0) {
        is_destroy = true;
    }
}

void Enemy7::UpdateDead(float) {
    Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.06f, false);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Destroy);
    Singleton<SEManager>::GetInstance()->PlaySE(SE_NAME::Dead1);
    Singleton<SEManager>::GetInstance()->ChangeSEVolume(SE_NAME::Dead1, 80);
    Singleton<ScoreData>::GetInstance()->AddScore(1800); // 中ボスらしく少し多め
    is_destroy = true;
    cleared = true;
}

// ===== 攻撃実装 =====

void Enemy7::Shot6WayFan() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();
    if (!player) return;

    // 3Way（ゆっくり・見やすい）
    Vector2D toP = player->GetLocation() - location;
    toP.Normalize();
    float base = atan2f(toP.y, toP.x);

    const int   n = 3;              // 3発
    const float half = Deg2Rad(22.0f); // 扇の半角を少し狭く
    const float speed = 120.0f;        // 遅め

    for (int i = 0; i < n; ++i) {
        float ang = base + (-half + (half * 2.0f) * (float)i / (float)(n - 1));
        Vector2D dir(cosf(ang), sinf(ang));
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dir * speed);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
}

void Enemy7::ShotRing12() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();

    // 6方向のゆっくりリング
    const int   n = 6;
    const float speed = 90.0f;  // さらに遅い

    for (int i = 0; i < n; ++i) {
        float ang = (DX_PI * 2.0f) * (float)i / (float)n;
        Vector2D dir(cosf(ang), sinf(ang));
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dir * speed);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
}


void Enemy7::ShotAimedTriplet() {
    auto* gm = Singleton<GameObjectManager>::GetInstance();
    auto* se = Singleton<SEManager>::GetInstance();
    if (!player) return;

    // 狙い方向に速度違いで3連（ズラして撃つ）
    Vector2D toP = player->GetLocation() - location;
    toP.Normalize();
    Vector2D dirs[3] = { toP, toP, toP };
    float spd[3] = { 140.0f, 180.0f, 220.0f };

    for (int i = 0; i < 3; ++i) {
        if (auto* b = gm->CreateObject<EnemyBullet2>(location)) {
            b->SetVelocity(dirs[i] * spd[i]);
            b->SetPlayer(player);
        }
    }
    se->PlaySE(SE_NAME::EnemyShot);
}

// ===== 補助 =====

bool Enemy7::CanShoot() const {
    // 画面内 & 十分フェードイン
    if (alpha < 120) return false;
    if (location.y < 0 || location.y > D_WIN_MAX_Y + 40) return false;
    return true;
}

void Enemy7::ForceRetreat() {
    if (pattern == Enemy7Pattern::Dead || pattern == Enemy7Pattern::Retreat) return;
    collision.is_blocking = false;
    collision.hit_object_type.clear();
    pattern = Enemy7Pattern::Retreat;
    pattern_timer = 0.0f;
}

void Enemy7::OnHitCollision(GameObjectBase* hit_object) {
    EnemyBase::OnHitCollision(hit_object);
    auto type = hit_object->GetCollision().object_type;

    if (type == eObjectType::eAttackShot)
    {
        hp -= 10;
    }

    if (type == eObjectType::eBeam)
    {
        beam_damage_timer += 1.0f / 60.0f;
        if (beam_damage_timer >= 0.15f)
        {
            hp -= 10;
            beam_damage_timer = 0.0f;
        }
    }
    // Beam継続ダメージの抑制など必要ならここで（今は基本は EnemyBase に任せる）
    // beam_damage_timer の活用も可
}

void Enemy7::PickNewWanderTarget() {
    float minx, maxx, miny, maxy;

    if (use_bounds) {
        minx = wander_bounds.xmin; maxx = wander_bounds.xmax;
        miny = wander_bounds.ymin; maxy = wander_bounds.ymax;
    }
    else {
        // 中央から ±350 の範囲
        float cx = D_WIN_MAX_X *0.5f;
        minx = cx - 350.0f;
        maxx = cx + 350.0f;

        // Yは浅め（従来通り 20?36% あたり）
        miny = D_WIN_MAX_Y * 0.20f;
        maxy = D_WIN_MAX_Y * 0.36f;
    }

    // 少しランダムに
    float nx = minx + (GetRand(1000) / 1000.0f) * (maxx - minx);
    float ny = miny + (GetRand(1000) / 1000.0f) * (maxy - miny);
    wander_target = Vector2D(nx, ny);
}
