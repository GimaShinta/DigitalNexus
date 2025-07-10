#pragma once
#include "EnemyBase.h"

enum class Enemy2Phase
{
    Appearing,
    Fighting
};

class Enemy2 : public EnemyBase
{
public:
    Enemy2();
    Enemy2(const Vector2D& pos);
    ~Enemy2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void EnableCircularMove(const Vector2D& center, float radius, float speed, float start_angle);
    void EnableRectangularLoopMove(bool clockwise = true);
    void EnableLoopMoveNow(); // Å© í«â¡
    bool IsAppearingFinished() const
    {
        return phase == Enemy2Phase::Fighting;
    }

protected:
    void Shot(float delta_second);

private:
    Enemy2Phase phase = Enemy2Phase::Appearing;
    float appear_timer = 0.0f;

    float scale = 0.2f;
    int alpha = 0;
    bool is_invincible = true;

    int sound_destroy;

    bool is_attacking = false;
    float attack_cooldown = 0.0f;
    int attack_pattern = 6;

    float spiral_timer = 0.0f;
    float spiral_total_time = 0.0f;

    void Pattern6(float delta_second);
    void Pattern7(float delta_second);

    Vector2D circular_center = { 640.0f, 360.0f };
    float circular_radius = 300.0f;
    float circular_angle = 0.0f;
    float circular_speed = 1.0f;
    bool is_circular_move = false;

    enum class MoveState
    {
        MoveDown,
        MoveRight,
        MoveUp,
        MoveLeft
    };

    bool is_rectangular_loop_move = false;
    bool is_clockwise = true;
    bool loop_move_enabled = false;  // Å© í«â¡
    MoveState move_state = MoveState::MoveDown;

    static constexpr float LOOP_TOP = 200.0f;
    static constexpr float LOOP_BOTTOM = 400.0f;
    static constexpr float LOOP_LEFT = 400.0f;
    static constexpr float LOOP_RIGHT = 880.0f;
};
