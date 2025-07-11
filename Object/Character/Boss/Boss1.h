#pragma once
#include "../Enemy/EnemyBase.h"

enum class BossPattern
{
    Entrance,
    GlitchWarp,
    CircleMove,
    ZigzagMove,
    RushDown,
    Hover,
    RandomMove
};

class Boss1 :
    public EnemyBase
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

protected:
    void Shot(float delta_second);

private:
    BossPattern pattern = BossPattern::Entrance;
    float pattern_timer = 0.0f;
    float pattern_change_timer = 0.0f;
    bool is_alive = true;

    bool floating_center_initialized = false;
    float floating_center_x = 0.0f;
    float floating_center_y = 0.0f;

    Vector2D start_location;
    bool has_shot = false;
    float after_shot_timer = 0.0f;
    float spawn_delay_timer = 0.0f;

    bool is_transforming = false;
    float transform_timer = 0.0f;
    float transform_duration = 2.0f;
    bool is_transformed = false;

    bool is_flashing = false;
    float flash_timer = 0.0f;
    bool visible = true;
    const float flash_interval = 0.2f;

    bool is_screen_flash = false;
    float screen_flash_timer = 0.0f;
    float screen_flash_duration = 0.3f;

    float life_timer = 0.0f;
    bool is_returning = false;
    bool is_leaving = false;
    Vector2D return_target = 0.0f;
    Vector2D original_location_before_return = 0.0f;
    float return_timer = 0.0f;
    const float return_duration = 2.0f;
    float stop_timer = 0.0f;
    float shot_timer = 0.0f;
    float stop_duration = 2.0f;

    std::vector<int> images;
    std::vector<int> images_a;
    std::vector<int> images_b;
    std::vector<int> anim_indices;

    void ChangePatternRandomly();

    int sound_destroy = NULL;

    // Entranceパターン専用：ワープフラグ
    bool entrance_warp_done[5] = { false };

    class Player* player = nullptr;

public:
    void SetPlayer(Player* p) { player = p; }
};

