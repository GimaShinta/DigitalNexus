#pragma once
#include "../../../Object/Character/Enemy/EnemyBase.h"
#include <vector>

class Player;
class Boss2RotatingPart;

class Boss2 : public EnemyBase
{
public:
    Boss2();
    ~Boss2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    bool GetIsAlive() const;
    void SetPlayer(Player* p)
    {
        player = p;
    }

    // パーツから参照する共通回転角
    float GetOrbitBaseAngle() const
    {
        return orbit_base_angle;
    }

    // ★追加：イントロ中かどうか（Stage2が演出遷移で使う）
    bool IsIntroActive() const
    {
        return intro_active;
    }

private:
    void Shot(float delta_second);

    // リング（円周）上の等間隔スロットを再計算
    void ReindexRing();

private:
    static constexpr int BOSS2_MAX_HP = 4000;

    // 状態
    bool  is_alive = true;
    bool  is_flashing = false;
    bool  visible = true;
    bool  pattern_toggle = false;
    bool  group_toggle = false;

    float flash_timer = 0.0f;
    float shot_timer = 0.0f;

    // 表示・アニメ
    std::vector<int> images;
    std::vector<int> images_a;
    std::vector<int> images_b;
    std::vector<int> anim_indices;
    int image = -1;

    // 破壊音（既存）
    int sound_destroy = -1;

    // 参照
    Player* player = nullptr;

    // --- 回転パーツ管理 ---
    std::vector<Boss2RotatingPart*> rotating_parts;

    // 共通回転
    float orbit_base_angle = 0.0f; // [deg]
    float orbit_speed_deg = 90.0f; // [deg/sec]
    float orbit_radius = 250.0f;

    // --- 登場演出 ---
    bool  intro_active = true; // イントロ中はShot停止
    float intro_timer = 0.0f;
    float appear_scale = 1.8f;
    int   intro_spawned_count = 0;
    float part_spawn_timer = 0.0f;
    float part_spawn_interval = 0.35f; // 1体ずつ出す間隔
};
