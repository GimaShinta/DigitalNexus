#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"

enum class Enemy2Mode {
    Zako3Like,  // �����F������ʂ�`���ē��B���ˌ������V���ޏ�
    LineRise,   // �ǉ��F��ɂȂ��ď��Ԃɉ�����o��
};

enum class Enemy2State {
    Waiting,    // LineRise�p�F�����őҋ@�i���ԑ҂��j
    Appearing,  // �o���A�j���iZako3���� / LineRise���ʁj
    Floating,   // �ӂ�ӂ�؍�
    Leaving,    // �ޏ�
};

class Enemy2 : public EnemyBase {
public:
    Enemy2();
    ~Enemy2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    // ����API�F�ʒu�Ǝ��ԁiappear_time�͍��񂿂��Ǝg�p�j
    void SetAppearParams(const Vector2D& start_pos_param, const Vector2D& end_pos_param, float appear_time, bool from_left);

    // �ǉ��F���[�h�ؑց����C�����o�p�p�����[�^
    void SetMode(Enemy2Mode m) { mode = m; }
    void SetLineParams(int index, int total, float interval_sec);

    // �I�v�V����
    void SetFlip(bool flip) { zako3_flip = flip; }
    void ShootToPlayer(float speed);

private:
    // ���[�h�Ə��
    Enemy2Mode  mode = Enemy2Mode::Zako3Like;
    Enemy2State state = Enemy2State::Appearing;

    // �^�C�}��
    float float_timer = 0.0f;       // ���V�E����
    float appear_timer = 0.0f;      // �o���A�j���p
    float appear_duration = 1.2f;   // �o�����ԁi�����1.2s�Ɂj
    float wait_timer = 0.0f;        // LineRise�ҋ@�^�C�}
    float wait_time = 0.0f;         // LineRise�ł̎����̊J�n�x��

    // ������
    float scale_min = 0.3f;
    float scale_max = 0.8f;
    float scale = 0.3f;
    float rotation = 0.0f;
    int   alpha = 0;

    // �z�u
    bool  is_from_left = true;
    bool  zako3_flip = false;
    Vector2D start_pos = 0.0f;
    Vector2D target_pos = 0.0f;
    Vector2D base_location = 0.0f;

    // ���C���o����i�K�v�ɉ����ė��p�j
    int   line_index = 0;           // �����̏��ԁi0-based�j
    int   line_total = 1;           // ����
    float line_interval = 0.5f;     // 1�̂��̊J�n�Ԋu

    // �摜�iZako3�n�j
    std::vector<int> zako3_images;
    int zako3_image = -1;

    // ����
    void Shot(float delta_second);
    template <typename T> T my_min(T a, T b) { return (a < b) ? a : b; }
};
