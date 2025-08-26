#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include <vector>

enum class Enemy4Pattern {
    Entrance,
    Battle,
    Retreat,
    Dead
};

class Enemy4 : public EnemyBase {
public:
    Enemy4();
    ~Enemy4();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPlayer(Player* p) { player = p; }
    void ForceRetreat();

private:
    // 内部更新
    void UpdateEntrance(float dt);
    void UpdateBattle(float dt);
    void UpdateRetreat(float dt);
    void UpdateDead(float dt);

    // 4Way 系のみ（視認性重視）
    void Shot4WayAim();             // 狙い基準の4Way
    void Shot4WayWide();            // ちょい広め4Way
    void Shot4WayStaggeredBurst();  // 交互オフセットの小バースト4Way


    // ユーティリティ
    static inline float Saturate(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
    static inline float EaseOutCubic(float t01) { float u = 1.0f - t01; return 1.0f - u * u * u; }
    static inline float Deg2Rad(float d) { return d * (DX_PI / 180.0f); }
    bool CanShoot() const;        // 画面内に居る＆十分フェードイン済み？

    // 表示/状態
    std::vector<int> images;
    int   image_index = 0;
    float anim_time = 0.0f;

    Enemy4Pattern pattern = Enemy4Pattern::Entrance;
    float pattern_timer = 0.0f;

    Vector2D enter_start = 0.0f;
    Vector2D target_pos = 0.0f;
    float    enter_time = 1.6f;
    float    scale_from = 4.0f;
    float    scale_to = 3.0f;
    float    scale_draw = 3.0f;
    int      alpha = 0;

    // 徘徊
    Vector2D wander_target = 0.0f;
    float    wander_timer = 0.0f;
    float    next_wander = 1.2f;
    float    noise_t = 0.0f;

    // 攻撃
    float    shot_timer = 0.0f;
    int      attack_mode = 0; // 0..2 の 3Wayパターンを回す

    bool     is_alive = false;

    // 被弾（Beamの継続ダメージ調整）
    void OnHitCollision(GameObjectBase* hit_object) override;
    float beam_damage_timer = 0.0f;

    // 難易度
    bool easy_mode = false;

    // 可視化補助：発射フラッシュ（短時間）
    mutable float shot_flash_t = 0.0f;

public:
    void SetEasyMode(bool on);
    bool IsCleared() const;
};
