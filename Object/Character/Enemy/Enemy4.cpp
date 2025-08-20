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

    // 大きめに（当たり判定も拡大）
    hp = 3000;
    is_destroy = false;
    is_alive = true;
    box_size = Vector2D(50, 50);     // だいぶ大きめ
    z_layer = 2;

    scale_from = 4.0f;   // ← デカく
    scale_to = 3.0f;   // ← 戦闘中も3倍で維持
    scale_draw = scale_from;

    // 画像（必要に応じ差し替え）
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy75_a.png",
        6, 6, 1, 40, 40);
    image_index = 0;
    image = images.empty() ? -1 : images[0];

    // 画面手前（上端のさらに手前）から“落ちてくる”
    enter_start = Vector2D(D_WIN_MAX_X * 0.5f, -240.0f);
    target_pos = Vector2D(D_WIN_MAX_X * 0.5f, D_WIN_MAX_Y * 0.32f);
    location = enter_start;

    alpha = 0;
    scale_draw = scale_from;        // 2.0 から開始（デカい）

    // 登場中は被弾なし → 戦闘移行時に有効化（Boss1 と同じ考え方）:contentReference[oaicite:3]{index=3}L18-L27
    collision.is_blocking = false;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.clear();

    // タイマー系
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

    // 画像アニメーション
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

    // 位置：上から目標へ
    location.y = enter_start.y + (target_pos.y - enter_start.y) * e;

    // スケール：2.0 → 1.2（“手前から奥へ”の遠近感）
    scale_draw = scale_from + (scale_to - scale_from) * e;

    // フェード：0 → 255
    alpha = (int)(255.0f * e);
    if (alpha > 255) alpha = 255;

    if (t >= 1.0f) {
        // 当たり判定を有効化（Boss1 と同じ思想）:contentReference[oaicite:4]{index=4}L18-L27
        collision.is_blocking = true;
        collision.object_type = eObjectType::eEnemy;
        collision.hit_object_type.clear();
        collision.hit_object_type.push_back(eObjectType::eAttackShot);
        collision.hit_object_type.push_back(eObjectType::eBeam);

        // バトルへ
        pattern = Enemy4Pattern::Battle;
        pattern_timer = 0.0f;
        shot_timer = 0.0f;

        // 徘徊の初期目標
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(200) - 100),
            D_WIN_MAX_Y * 0.28f + (GetRand(140) - 70)
        );
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f); // 1.0?1.8秒
    }
}

void Enemy4::UpdateBattle(float dt) {
    // ===== 不規則徘徊（ランダム目標へスムーズ追従＋ノイズ揺れ）=====
    wander_timer += dt;
    noise_t += dt;

    if (wander_timer >= next_wander) {
        wander_timer = 0.0f;
        next_wander = 1.0f + (GetRand(80) / 100.0f); // 1.0?1.8秒
        wander_target = Vector2D(
            D_WIN_MAX_X * 0.5f + (GetRand(280) - 140),          // 横に大きめ
            D_WIN_MAX_Y * 0.28f + (GetRand(180) - 90)           // 縦もランダム
        );
    }

    // 目標にスムーズ追従（イージング）
    Vector2D to = wander_target - location;
    location += to * (2.8f * dt); // 追従係数（調整可）

    // 小刻みノイズ（見た目の不規則さ）
    float nX = sinf(noise_t * 2.3f) * 6.0f + sinf(noise_t * 1.7f + DX_PI / 3) * 4.0f;
    float nY = cosf(noise_t * 2.0f) * 5.0f + sinf(noise_t * 2.6f + DX_PI / 5) * 3.0f;
    location.x += nX * dt * 4.0f;
    location.y += nY * dt * 4.0f;

    // ===== 攻撃（激しめローテーション）=====
    shot_timer += dt;

    // HP割合（攻撃感度を可変）
    float hp_ratio = (float)hp / 2300.0f;
    if (hp_ratio < 0.0f) hp_ratio = 0.0f;
    float interval = 0.95f * hp_ratio + 0.35f; // HP減で短く（0.35?1.3s）

    if (shot_timer >= interval) {
        shot_timer = 0.0f;

        switch (attack_mode % 4) {
        case 0:  ShotFanBurst();   break;   // 広角3連
        case 1:  ShotFastSpiral(); break;   // 高速スパイラル
        case 2:  ShotAllRange();   break;   // 全方位リング
        default: ShotCrossFan();   break;   // 十字＋扇
        }
        attack_mode++;
    }

    // 撃破
    if (hp <= 0) {
        pattern = Enemy4Pattern::Dead;
        pattern_timer = 0.0f;
    }
}

void Enemy4::ForceRetreat() {
    if (pattern == Enemy4Pattern::Dead || pattern == Enemy4Pattern::Retreat) return;

    // 無敵化・被弾不可
    collision.is_blocking = false;
    collision.hit_object_type.clear();

    // 退場へ
    pattern = Enemy4Pattern::Retreat;
    pattern_timer = 0.0f;
}

void Enemy4::UpdateRetreat(float dt) {
    // 上方向へスッと退場（少し加速感）
    float up_speed = 260.0f + 120.0f * (pattern_timer); // 徐々に加速
    location.y -= up_speed * dt;

    // フェードアウト（1.2秒程度で透明に）
    int a = 255 - (int)(pattern_timer * (255.0f / 1.2f));
    if (a < 0) a = 0;
    alpha = a;

    // 画面外に抜けたら消滅
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

// ===== 攻撃パターン =====
// Boss1 の発想（スパイラル / 全方位 / 扇）を EnemyBullet2 で再構成:contentReference[oaicite:5]{index=5}L112-L140:contentReference[oaicite:6]{index=6}L142-L161:contentReference[oaicite:7]{index=7}L163-L190
void Enemy4::ShotFanBurst() {
    // 下方向広角を3連発（小間隔）
    auto objm = Singleton<GameObjectManager>::GetInstance();
    const int volley = 3;
    const int count_per = 11;
    const float spread = 80.0f; // 広角
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
        // 連射の間隔を短く見せるため、少しだけ押し出す
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
    base += 28.0f; // 高速回転
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Enemy4::ShotAllRange() {
    auto objm = Singleton<GameObjectManager>::GetInstance();
    const int count = 28; // 全方位ばらまき
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

    // 十字のみ（※ 扇の追加発射は削除）
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

    // 通常弾
    if (type == eObjectType::eAttackShot)
    {
        hp -= 10;
    }

    // ビーム（接触中、一定間隔で減算）
    if (type == eObjectType::eBeam)
    {
        beam_damage_timer += 1.0f / 60.0f; // 1フレーム=1/60秒換算
        if (beam_damage_timer >= 0.35f)
        {
            hp -= 10;
            beam_damage_timer = 0.0f;
        }
    }
}


void Enemy4::Draw(const Vector2D& screen_offset) const {
    if (image < 0) return;

    // ===== 落下中の速度線（ストリーク）で“落ちる”を強調 =====
    if (pattern == Enemy4Pattern::Entrance) {
        float t = Saturate(pattern_timer / enter_time);
        int streaks = 8;
        int a = (int)(120.0f * (1.0f - t)); // 最初だけ強めに

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
    DrawRotaGraph(
        (int)(location.x - screen_offset.x),
        (int)(location.y - screen_offset.y),
        scale_draw,    // ← 今は 3.0f?4.0f になる
        0.0f,
        image,
        TRUE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // === 小型HPゲージ（中ボス追従） ===
    {
        // hp比（0.0～1.0）
        float hp_ratio = (float)hp / 3000.0f;
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;
        if (hp_ratio > 1.0f) hp_ratio = 1.0f;

        // フェード係数（登場直後は少し控えめ）
        float fade_ratio = (pattern_timer < 0.0f) ? 0.0f
            : ((pattern_timer > 1.2f) ? 1.0f : (pattern_timer / 1.2f));
        int bar_alpha = (int)(fade_ratio * 200);

        // 位置・サイズ（敵の頭上）
        const int bar_width = 140;
        const int bar_height = 5;
        const int bar_x = (int)(location.x - screen_offset.x) - bar_width / 2;
        const int bar_y = (int)(location.y - screen_offset.y) - (int)(scale_draw * 24.0f) - 8;

        // わずかな揺らぎ（見やすさの範囲に抑える）
        float wave = sinf(GetNowCount() * 0.01f) * 0.8f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, bar_alpha);

        // 背景
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + bar_width, bar_y + bar_height + (int)wave,
            GetColor(40, 40, 40), TRUE);

        // 本体（赤系：白っぽ過ぎないよう調整）
        int bar_color = GetColor(230, 70, 70);
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + (int)(bar_width * hp_ratio), bar_y + bar_height + (int)wave,
            bar_color, TRUE);

        // 枠
        DrawBox(bar_x, bar_y + (int)wave,
            bar_x + bar_width, bar_y + bar_height + (int)wave,
            GetColor(180, 180, 180), FALSE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Enemy4::Finalize() {
    // 特になし
}
