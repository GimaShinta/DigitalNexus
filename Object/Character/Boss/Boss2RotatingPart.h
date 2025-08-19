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

    // Boss�Ɖ�]�����̃Z�b�g�A�b�v
    void SetUp(Boss2* boss, float radius, float angle_offset);
    // �O���[�o���p�x�i�I�t�Z�b�g���݁j���擾
    float GetAngleGlobal() const;

    // �e�����S���̃t���O�ݒ�itrue: ���̃p�[�c�͒e������ / false: �����Ȃ��j
    void SetIsBulletPart(bool value)
    {
        is_bullet_part = value;
    }

private:
    // �e���p�^�[��
    enum class BulletPattern
    {
        Spiral, NWay, Ring
    };

    // �֗�: �e���ꔭ���ispeed �� [pixel/sec]�j
    void FireBullet(const Vector2D& dir, float speed);

private:
    Boss2* boss = nullptr;
    float radius = 0.0f;
    float angle_offset = 0.0f;
    float angle = 0.0f;           // [deg]
    float angle_speed = 90.0f;    // [deg/sec]

    int image = -1;

    // �e������
    bool  is_bullet_part = false;
    float shot_timer = 0.0f;      // ���ˊԊu�p
    float mode_timer = 0.0f;      // �p�^�[���ؑ֗p
    float spiral_phase = 0.0f;    // �X�p�C�����p�i[rad]�j
    BulletPattern pattern = BulletPattern::Spiral;

    // �����p�p�����[�^�i�K�v�Ȃ�D�݂ŕύX�j
    float interval_spiral = 0.08f;    // �X�p�C�����̘A�ˊԊu
    float interval_nway = 0.60f;     // NWay �̊Ԋu
    float interval_ring = 2.00f;     // �����O�����̊Ԋu
    float pattern_cycle = 6.00f;     // ���b���ƂɃp�^�[���ؑ�

    int   nway_count = 5;         // NWay �̒e���i������j
    float nway_spread_deg = 40.0f;     // NWay �̊g�U�p�i�x�j
    int   ring_count = 24;        // �����O�̒e��

    float speed_spiral = 240.0f;    // ���ꂼ��̒e�� [px/s]
    float speed_nway = 260.0f;
    float speed_ring = 220.0f;
};
