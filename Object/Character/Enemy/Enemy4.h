#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include <vector>

enum class Enemy4Pattern {
    Entrance,   // 画面手前から落ちる（スケール縮小＋フェードイン）
    Battle,     // 不規則に動きながら激しめ弾幕
    Retreat,   // 退場
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

    // ★ 追加API：退場開始を外部から指示
    void ForceRetreat();

private:
    // 内部更新
    void UpdateEntrance(float dt);
    void UpdateBattle(float dt);
    void UpdateDead(float dt);
    void UpdateRetreat(float dt);   // ★ 追加

    // 攻撃
    void ShotFanBurst();      // 下方向広角を連射（3連）
    void ShotFastSpiral();    // 高速スパイラル
    void ShotAllRange();      // 全方位リング
    void ShotCrossFan();      // 十字＋扇の合わせ技

    // 落下演出：0..1 サチュレート / イージング
    static inline float Saturate(float t) {
        return (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    }
    static inline float EaseOutCubic(float t01) {
        float u = 1.0f - t01;
        return 1.0f - u * u * u;
    }
    static inline float Deg2Rad(float d) { return d * (DX_PI / 180.0f); }

    // 表示
    std::vector<int> images;
    int   image_index = 0;
    float anim_time = 0.0f;

    // パターン管理
    Enemy4Pattern pattern = Enemy4Pattern::Entrance;
    float pattern_timer = 0.0f;

    // 落下演出
    Vector2D enter_start = 0.0f;
    Vector2D target_pos = 0.0f;
    float    enter_time = 1.6f;   // 1.6秒で収束
    float    scale_from = 2.0f;   // かなり大きく
    float    scale_to = 1.2f;   // 大きめをキープ
    float    scale_draw = 1.2f;   // 現在スケール
    int      alpha = 0;      // フェードイン

    // バトル中の徘徊
    Vector2D wander_target = 0.0f;
    float    wander_timer = 0.0f;
    float    next_wander = 1.2f;  // 1.0?1.8秒で更新（可変）
    float    noise_t = 0.0f;

    // 攻撃
    float    shot_timer = 0.0f;
    int      attack_mode = 0;      // 0..3 パターン

    bool is_alive = false;

    // 追加（public内）
    void OnHitCollision(GameObjectBase* hit_object) override;

    // 追加（privateでもOK）
    float beam_damage_timer = 0.0f;

    
};
