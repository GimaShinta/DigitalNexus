#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include <vector>

// Enemy7：Stage2 中ボス（Entrance→Battle→Retreat→Dead）
enum class Enemy7Pattern {
    Entrance,
    Battle,
    Retreat,
    Dead
};

struct RectF {
    float xmin, ymin, xmax, ymax;
};

class Enemy7 : public EnemyBase {
public:
    Enemy7();
    ~Enemy7();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPlayer(Player* p) { player = p; }
    void ForceRetreat();          // 強制退場（時間切れなど）
    void SetEasyMode(bool on) { easy_mode = on; }
    bool IsCleared() const { return cleared; }



private:
    // 内部更新
    void UpdateEntrance(float dt);
    void UpdateBattle(float dt);
    void UpdateRetreat(float dt);
    void UpdateDead(float dt);

    // 攻撃（視認性重視）
    void Shot6WayFan();           // 広がる6Way（遅め）
    void ShotRing12();            // 12方向リング（中速）
    void ShotAimedTriplet();      // 狙い基準3連（遅→中→速）

    // ユーティリティ
    static inline float Saturate(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
    static inline float EaseOutCubic(float t01) { float u = 1.0f - t01; return 1.0f - u * u * u; }
    static inline float Deg2Rad(float d) { return d * (DX_PI / 180.0f); }
    bool CanShoot() const;

private:
    // 表示
    std::vector<int> images;
    int   image_index = 0;
    float anim_time = 0.0f;

    Enemy7Pattern pattern = Enemy7Pattern::Entrance;
    float pattern_timer = 0.0f;

    // Entrance
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
    int      attack_mode = 0; // 0..2 を回す

    // 追加
    bool     easy_mode = false;
    bool     cleared = false;   // 撃破→true
    mutable float shot_flash_t = 0.0f;

    // Beam継続ダメージの調整例（必要なら）
    float beam_damage_timer = 0.0f;

    // 当たり判定反応
    void OnHitCollision(GameObjectBase* hit_object) override;

    // 徘徊範囲
    RectF wander_bounds{ 0,0,0,0 };
    bool  use_bounds = false;
    const char* label = nullptr; // "LEFT"/"RIGHT"など（任意）

    Vector2D anchor_pos = 0.0f;  // 入場完了時の基準位置
    float    orbit_t = 0.0f;     // 軌道用の位相
    float    orbit_speed = 0.8f; // 0.6～1.0程度で個体差
    float    orbit_rx = 60.0f;   // X方向に揺れる半径（小さめ）
    float    orbit_ry = 22.0f;   // Y方向に揺れる半径（かなり小さめ）
    float    bob_amp_y = 4.0f;   // ごく小さな縦のゆらぎ
    float    bob_spd = 1.3f;   // 縦ゆらぎの速さ

    // 内部：範囲内で新しい目標を作る
    void PickNewWanderTarget();

public:
    // 追加：徘徊範囲を指定
    void SetWanderBounds(const RectF& r) { wander_bounds = r; use_bounds = true; }
    // 追加：左右どちら用か（任意のラベル）を記録しておくとデバッグ便利
    void SetLabel(const char* t) { label = t; }
};
