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

    // Bossと回転条件
    void SetUp(Boss2* boss, float radius, float angle_offset);

    // スロット情報（円周上の等間隔配置）
    void SetRingParams(int index, int total)
    {
        ring_index = index; ring_total = (total <= 0 ? 1 : total);
    }
    // 弾撃ち担当
    void SetIsBulletPart(bool v)
    {
        is_bullet_part = v;
    }

    // グローバル角度（共有角＋スロット角）
    float GetAngleGlobal() const;

    // ===== パターン制御 =====
    enum class BulletPattern
    {
        Spiral,       // 位相回転スパイラル
        NWay,         // 外向きNWay
        Ring,         // その場リング
        Pendulum,     // サイン掃きNWay（左右スウィープ）
        Burst,        // 連続リング（数波）
        SemiHoming,   // 追尾（位相遅延の半追尾）
        DecelBurst,   // 減速→停止→放射
        TimedSplit,    // 時限分裂（一定時間後にリングへ分裂）
        SimpleAlternate,
        RotateShot,
        VShot,
        RhythmShot
    };

    void ForcePattern(BulletPattern p)
    {
        pattern = p; ResetPatternState();
    }
    void NextPattern();

    // パターンサイクル（自動切替秒）
    void SetPatternCycleSeconds(float s)
    {
        if (s > 0.0f) pattern_cycle = s;
    }

    // ===== HP（残り体力）による自動切替 =====
    // 例） 0.75, 0.50, 0.25 未満になったら、それぞれ別パターンへ切替
    void EnableHpDrivenPhases(bool enable)
    {
        hp_phase_enable = enable;
    }
    void SetHpPhaseThresholds(float t1, float t2, float t3)
    {
        hp_t1 = t1; hp_t2 = t2; hp_t3 = t3;
    }

private:
    // 弾を撃つ（speed: px/sec）
    void FireBullet(const Vector2D& dir, float speed);
    static Vector2D SafeNormalize(const Vector2D& v);
    void ResetPatternState();
    void UpdateHpDrivenPattern();   // HPを見てパターン切替
    float GetBossHpRate() const;    // [0.0,1.0] 残HP割合（Boss2のAPIに合わせて1行調整）

    // 追尾で使う：プレイヤー取得（Boss2やGOMから取れる方法に合わせて1行調整）
    bool TryGetPlayerPos(Vector2D& out) const;

private:
    Boss2* boss = nullptr;
    float  radius = 0.0f;
    float  angle_offset = 0.0f; // 未使用（互換保持）
    int    image = -1;

    // 配置
    int ring_index = 0;
    int ring_total = 1;

    // 弾幕制御
    bool  is_bullet_part = false;
    float shot_timer = 0.0f;
    float mode_timer = 0.0f;
    float spiral_phase = 0.0f;
    BulletPattern pattern = BulletPattern::Spiral;

    // 共通パラメータ
    float pattern_cycle = 6.0f; // 自動切替

    // Spiral
    float interval_spiral = 0.25f;
    float speed_spiral = 240.0f;

    // NWay
    float interval_nway = 0.6f;
    int   nway_count = 5;
    float nway_spread_deg = 40.0f;
    float speed_nway = 260.0f;

    // Ring
    float interval_ring = 1.6f;
    int   ring_count = 24;
    float speed_ring = 220.0f;

    // Pendulum
    float interval_pendulum = 0.12f;
    float pendulum_period_sec = 3.0f;
    float pendulum_amplitude_deg = 60.0f;
    int   pendulum_way = 7;
    float speed_pendulum = 240.0f;
    float pendulum_time = 0.0f;

    // Burst
    float interval_burst_cool = 2.4f;   // 1.8f -> 2.4f  波開始のクールタイムを長めに
    int   burst_waves = 2;              // 3   -> 2      波数を減らす
    float burst_inner_interval = 0.12f; // 0.08f -> 0.12f 同一波内の発射間隔を広げる
    int   burst_way = 8;                // 12  -> 8      1波あたりの弾数を減らす
    float speed_burst = 220.0f;         // 230 -> 220    速度を少し落として圧迫感も軽減
    int   burst_left = 0;
    float burst_inner_timer = 0.0f;


    // ===== NEW: SemiHoming（半追尾） =====
    float interval_semihoming = 0.10f;
    float speed_semihoming = 250.0f;
    // “半追尾”は、ターゲット角をローパス（遅延追従）する
    float sh_target_angle_rad = 0.0f;
    float sh_current_angle_rad = 0.0f;
    float sh_follow_k = 0.12f; // 0.0～1.0（大きいほど追従が速い）

    // ===== NEW: DecelBurst（減速→停止→放射） =====
    float interval_decelburst = 0.20f;
    float db_initial_speed = 320.0f;
    float db_decel_per_sec = 480.0f;  // 減速量[px/s^2]
    float db_min_speed = 20.0f;   // これ以下で「停止」扱い
    int   db_ring_way = 16;      // 停止後に放つリング弾数
    float db_ring_speed = 220.0f;

    // ===== NEW: TimedSplit（時限分裂） =====
    float interval_tsplit = 0.25f;
    float ts_initial_speed = 280.0f;
    float ts_split_time = 1.20f;   // 何秒後に分裂するか
    int   ts_ring_way = 10;
    float ts_ring_speed = 230.0f;

    // HPフェーズ
    bool  hp_phase_enable = true;
    float hp_t1 = 0.75f, hp_t2 = 0.50f, hp_t3 = 0.25f;
    int   hp_phase = 0; // 0:>t1, 1:(t2,t1], 2:(t3,t2], 3:<=t3

    // Boss2RotatingPart.h
    float rotation_angle = 0.0f;

};
