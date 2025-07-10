#pragma once

#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet1.h"

// �U�R�G�̍s���p�^�[����`
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
    RetreatUp,     // �� �ǉ��F������ɑޏ�
    SlowDownThenMove,   // �� �V�p�^�[��
    PauseThenRush,      // �� �V�p�^�[��
    RotateAndShoot,
    Count,
    SlowInShootOut,
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

private:
    float custom_ascend_y = 0.0f; // �㏸���܂�Ԃ��ʒu
    float custom_stop_y = 200.0f;   // �~������~�E�U���ʒu
    Player* player;

public:
    void SetAscendY(float y) { custom_ascend_y = y; }
    void SetStopY(float y) { custom_stop_y = y; }    // �� �V�����ǉ�



    void SetPattern(Enemy1Pattern new_pattern);
    Enemy1Pattern GetPattern() const
    {
        return pattern;
    }  // �� �ǉ�

protected:
    void Shot(float delta_second);

private:
    Enemy1Pattern pattern = Enemy1Pattern::MoveStraight;
    float pattern_timer = 0.0f;

    Vector2D start_location;
    bool is_returning = false;
    bool has_shot = false;
    float after_shot_timer = 0.0f;
    float spawn_delay_timer = 0.0f;
    float scale = 1.0f;
    float shot_timer = 0.0f;

    std::vector<int> images;
    std::vector<int> images_a;
    std::vector<int> images_b;
    std::vector<int> anim_indices;

    void ChangePatternRandomly();


    //�q�b�g��
    int sound_hit = -1;
    int sound_destroy = -1;
    int se_start = -1;
};
