#pragma once
#include "../../../Object/Character/Enemy/EnemyBase.h"

class Boss2;

class Boss2RotatingPart : public EnemyBase
{
public:
    Boss2RotatingPart();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;

    // Bossと回転条件のセットアップ
    void SetUp(Boss2* boss, float radius, float angle_offset);
    // グローバル角度（オフセット込み）を取得
    float GetAngleGlobal() const;

    // 弾撃ち担当のフラグ設定（true: このパーツは弾を撃つ / false: 撃たない）
    void SetIsBulletPart(bool value)
    {
        is_bullet_part = value;
    }

private:
    // 弾幕パターン
    enum class BulletPattern
    {
        Spiral, NWay, Ring
    };

    // 便利: 弾を一発撃つ（speed は [pixel/sec]）
    void FireBullet(const Vector2D& dir, float speed);

private:
    Boss2* boss = nullptr;
    float radius = 0.0f;
    float angle_offset = 0.0f;
    float angle = 0.0f;           // [deg]
    float angle_speed = 90.0f;    // [deg/sec]

    int image = -1;

    // 弾幕制御
    bool  is_bullet_part = false;
    float shot_timer = 0.0f;      // 発射間隔用
    float mode_timer = 0.0f;      // パターン切替用
    float spiral_phase = 0.0f;    // スパイラル角（[rad]）
    BulletPattern pattern = BulletPattern::Spiral;

    // 調整用パラメータ（必要なら好みで変更）
    float interval_spiral = 0.08f;    // スパイラルの連射間隔
    float interval_nway = 0.60f;     // NWay の間隔
    float interval_ring = 2.00f;     // リング生成の間隔
    float pattern_cycle = 6.00f;     // 何秒ごとにパターン切替

    int   nway_count = 5;         // NWay の弾数（奇数推奨）
    float nway_spread_deg = 40.0f;     // NWay の拡散角（度）
    int   ring_count = 24;        // リングの弾数

    float speed_spiral = 240.0f;    // それぞれの弾速 [px/s]
    float speed_nway = 260.0f;
    float speed_ring = 220.0f;
};
