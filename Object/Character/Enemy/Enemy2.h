#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"

enum class Enemy2State
{
    Appearing,
    Floating,
    Leaving
};

enum class Enemy2MoveMode
{
    Normal,         // �ʏ�i�Y���j
    ChaseFeint,     // �ǔ��t�F�C���g
    DoubleHelix     // �_�u���w���b�N�X
};

class Enemy2 : public EnemyBase
{
public:
    Enemy2();
    ~Enemy2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetAppearParams(const Vector2D& start, const Vector2D& end, float time);
    void SetPlayer(Player* p) { player = p; }

    // ���[�h�w�胁�\�b�h
    void SetChaseAndFeint(bool from_left, Player* p);
    void SetDoubleHelixMove(bool from_left, int index);

protected:
    void Shot(float speed);

private:
    std::vector<int> images;
    int image = -1;

    Enemy2State state = Enemy2State::Appearing;
    Enemy2MoveMode move_mode = Enemy2MoveMode::Normal;

    // �o���p�p�����[�^
    Vector2D start_location = 0.0f;
    Vector2D target_location = 0.0f;
    Vector2D base_location = 0.0f;

    float appear_timer = 0.0f;
    float float_timer = 0.0f;
    float appear_duration = 1.5f;

    // �A�j���[�V����
    float animation_time = 0.0f;
    int animation_count = 0;

    // �`�扉�o
    float scale = 1.0f;
    float scale_min = 0.4f;
    float scale_max = 1.6f;
    int alpha = 0;

    // �ǔ��t�F�C���g�p
    bool feint_from_left = true;
    Player* player_ref = nullptr;
    bool feint_triggered = false;

    // �_�u���w���b�N�X�p
    bool helix_from_left = true;
    float helix_angle = 0.0f;
    float helix_radius = 180.0f;
    int helix_order = 0;
};
