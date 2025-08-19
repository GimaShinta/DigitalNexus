 #pragma once

#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet3.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet5.h"

enum class Enemy1Pattern
{
    MoveStraight,
    RightMove,
    LeftMove,
    ZIgzag,
    SideAppearAndShoot,
    MoveAndStopShoot,
    MoveThenDiagonal,
    Formation,
    DiveOnce,
    ArcMoveAndStop,
    DepthAppear,
    RetreatUp,
    SlowDownThenMove,
    PauseThenRush,
    RotateAndShoot,
    Count,
    SlowInShootOut,
};

enum class Enemy1State {
    Appearing,
    Floating,
    Leaving
};


class Enemy1 : public EnemyBase
{
public:
    Enemy1();
    Enemy1(const Vector2D& pos);
    ~Enemy1();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    //void SetAscendY(float y) { custom_ascend_y = y; }
    //void SetStopY(float y) { custom_stop_y = y; }
    //void SetPattern(Enemy1Pattern new_pattern);
    //Enemy1Pattern GetPattern() const { return pattern; }

    void SetAppearParams(const Vector2D& start, const Vector2D& end, float time);
    void SetPlayer(Player* p) { player = p; }

protected:
    void Shot(float delta_second);

//private:
//    Enemy1Pattern pattern = Enemy1Pattern::MoveStraight;
//    Enemy1State state = Enemy1State::Appearing;
//
//    Player* player = nullptr;
//
//    Vector2D start_location = 0.0f;
//    Vector2D target_location = 0.0f;
//    Vector2D base_location = 0.0f;
//
//    float pattern_timer = 0.0f;
//    float float_timer = 0.0f;
//    float appear_timer = 0.0f;
//    float appear_duration = 1.5f;
//
//    float custom_ascend_y = 0.0f;
//    float custom_stop_y = 200.0f;
//
//    float after_shot_timer = 0.0f;
//    float spawn_delay_timer = 0.0f;
//    float shot_timer = 0.0f;
  float scale = 1.0f;
//    int alpha = 0;
//
//    bool is_returning = false;
//    bool has_shot = false;

private:
    std::vector<int> images;
    std::vector<int> images_a;
    std::vector<int> images_b;
    std::vector<int> anim_indices;

   // void ChangePatternRandomly();

    int sound_hit = -1;
    int sound_destroy = -1;
    int se_start = -1;

    float animation_time = 0.0f;
    int animation_count = 0;

private:
    Enemy1State state = Enemy1State::Appearing;

    Vector2D start_location = 0.0f;
    Vector2D target_location = 0.0f;
    Vector2D base_location = 0.0f;

    float appear_timer = 0.0f;
    float float_timer = 0.0f;
    float appear_duration = 1.2f;

    float scale_min = 0.3f;
    float scale_max = 1.2f;
    int alpha = 0;

    float leave_delay = 3.0f;


    template <typename T>
    T my_min(T a, T b) {
        return (a < b) ? a : b;
    }

};
