#pragma once
#include "../../Object/GameObjectBase.h"

// ===================== FollowBeam =====================
class FollowBeam : public GameObjectBase
{
public:
    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void OnHitCollision(GameObjectBase* hit_object) override;

    void SetFollowTarget(GameObjectBase* target, const Vector2D& direction);
    void SetBeamTarget(Vector2D target_position);
    void SetFollowAndLookTarget(GameObjectBase* follow, GameObjectBase* look, float max_length);
    void SetBeamThickness(float thickness);
    bool GetIsDestroy() const
    {
        return is_destroy;
    }

private:
    GameObjectBase* follow_target = nullptr;
    GameObjectBase* look_target = nullptr;
    Vector2D dir;
    Vector2D target_pos;
    float length = 600.0f;
    float life_time = 1.0f;
    float elapsed_time = 0.0f;
    float beam_thickness = 3.0f;
};

// ===================== LinkedBeam =====================
class LinkedBeam : public GameObjectBase
{
public:
    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;

    void SetEndpoints(GameObjectBase* a, GameObjectBase* b);
    void SetThickness(float thickness);
    void SetLifeTime(float time);

private:
    GameObjectBase* part_a = nullptr;
    GameObjectBase* part_b = nullptr;
    float beam_thickness = 6.0f;
    float life_time = 1.5f;
    float elapsed_time = 0.0f;
};

// ===================== LinkedWarningBeam =====================
class LinkedWarningBeam : public GameObjectBase
{
public:
    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;

    void SetEndpoints(GameObjectBase* a, GameObjectBase* b);
    void SetLifeTime(float time);

private:
    GameObjectBase* part_a = nullptr;
    GameObjectBase* part_b = nullptr;
    float elapsed_time = 0.0f;
    float warning_time = 1.0f;
};

// ===================== WarningBeam =====================
class WarningBeam : public GameObjectBase
{
public:
    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;

    void SetUp(const Vector2D& direction, float length, float lifetime);
    void SetFollowTarget(GameObjectBase* target);
    void SetFollowAndLookTarget(GameObjectBase* follow, GameObjectBase* look);

private:
    Vector2D dir;
    float beam_length = 800.0f;
    float elapsed_time = 0.0f;
    float life_time = 0.5f;
    GameObjectBase* follow_target = nullptr;
    GameObjectBase* look_target = nullptr;
};

// ===================== TriangleWarningBeam =====================
class TriangleWarningBeam : public GameObjectBase
{
public:
    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;

    void SetEndpoints(GameObjectBase* a, GameObjectBase* b);
    void SetLifeTime(float time);

private:
    GameObjectBase* part_a = nullptr;
    GameObjectBase* part_b = nullptr;
    float elapsed_time = 0.0f;
    float warning_time = 0.6f;
};

