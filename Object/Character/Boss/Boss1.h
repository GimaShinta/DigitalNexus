#pragma once
#include "../Enemy/EnemyBase.h"

enum class BossPattern
{
    Entrance,
    Hovering,
    Dead
};

class Boss1 : public EnemyBase
{
public:
    Boss1();
    ~Boss1();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPattern(BossPattern new_pattern);
    bool GetIsAlive() const;

    void OnHitCollision(GameObjectBase* hit_object) override;


private:
    // 行動パターン更新
    void UpdateEntrance(float delta_second);
    void UpdateHovering(float delta_second);

    // 攻撃パターン
    void ShotSpiral(float delta_second);
    void ShotAllRange();
    void ShotCrossShot();
    void ShotFanWide();  // ★弾幕系パターン
    void ShotWaveBullets();          // ★追加パターン
    void ShotTripleSpread();         // ★追加パターン
    void ShotFastSpiral(float delta_second); // ★追加パターン
    void ExplosionEffect(float delta_second);

private:
    BossPattern pattern = BossPattern::Entrance;
    bool is_alive = true;

    // パターン管理
    float pattern_timer = 0.0f;
    int attack_mode = 0;

    // 移動・登場演出
    Vector2D target_pos;
    Vector2D velocity;
    float move_timer = 0.0f;

    // 攻撃管理
    float shot_timer = 0.0f;

    // 演出
    float alpha = 0.0f;

    // 爆発管理
    bool explosions_started = false;
    float explosion_timer = 0.0f;
    int explosion_count = 0;
    bool final_explosion_done = false;

    // 画像・音
    std::vector<int> images;
    int image = 0;
    int sound_destroy = -1;

    float aura_timer = 0.0f;
    float ripple_timer = 0.0f;

    float beam_damage_timer = 0.0f;
    const int BOSS1_MAX_HP = 8000;

   


};
