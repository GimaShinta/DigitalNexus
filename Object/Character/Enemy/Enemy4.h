#pragma once
#include "EnemyBase.h"
#include <vector>

enum class Enemy4Mode {
    Zako3,   // Enemy3(Zako3)�݊�
};

enum class Enemy4State {
    Appearing,   // ����o���i��O�֊�鉉�o�j
    Floating,    // �������h��đ؋�
    Leaving,     // ��֑ޏ�
};

class Enemy4 : public EnemyBase {
public:
    Enemy4();
    ~Enemy4();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetAppearParams(const Vector2D& start_pos_param,
        const Vector2D& end_pos_param,
        float appear_time,
        bool from_left);
    void SetMode(Enemy4Mode m) { mode = m; }
    void SetFlip(bool flip) { zako3_flip = flip; }

    // Zako3�Ɠ����g�v���C���[�ֈꔭ�h
    void ShootToPlayer(float speed);

private:
    // ���[�h/�X�e�[�g
    Enemy4Mode  mode = Enemy4Mode::Zako3;
    Enemy4State state = Enemy4State::Appearing;

    // ���Ԍn
    float float_timer = 0.0f; // ���V�E�����J�E���g
    float appear_timer = 0.0f; // �o��A�j��
    float appear_duration = 1.2f; // ���B�܂�

    // ������
    float scale_min = 0.3f;
    float scale_max = 0.8f;
    float scale = 0.3f;
    float rotation = 0.0f;
    int   alpha = 0;
    bool  zako3_flip = false;
    bool  is_from_left = true;

    // �ʒu
    Vector2D start_pos = 0.0f;
    Vector2D target_pos = 0.0f;
    Vector2D base_loc = 0.0f;

    // �摜
    std::vector<int> zako3_images;
    int zako3_image = -1;

    template <typename T> T my_min(T a, T b) { return (a < b) ? a : b; }
};
