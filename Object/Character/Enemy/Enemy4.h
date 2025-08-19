#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include <vector>

enum class Enemy4Pattern {
    Entrance,   // ��ʎ�O���痎����i�X�P�[���k���{�t�F�[�h�C���j
    Battle,     // �s�K���ɓ����Ȃ��猃���ߒe��
    Retreat,   // �ޏ�
    Dead
};

class Enemy4 : public EnemyBase {
public:
    Enemy4();
    ~Enemy4();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPlayer(Player* p) { player = p; }

    // �� �ǉ�API�F�ޏ�J�n���O������w��
    void ForceRetreat();

private:
    // �����X�V
    void UpdateEntrance(float dt);
    void UpdateBattle(float dt);
    void UpdateDead(float dt);
    void UpdateRetreat(float dt);   // �� �ǉ�

    // �U��
    void ShotFanBurst();      // �������L�p��A�ˁi3�A�j
    void ShotFastSpiral();    // �����X�p�C����
    void ShotAllRange();      // �S���ʃ����O
    void ShotCrossFan();      // �\���{��̍��킹�Z

    // �������o�F0..1 �T�`�����[�g / �C�[�W���O
    static inline float Saturate(float t) {
        return (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    }
    static inline float EaseOutCubic(float t01) {
        float u = 1.0f - t01;
        return 1.0f - u * u * u;
    }
    static inline float Deg2Rad(float d) { return d * (DX_PI / 180.0f); }

    // �\��
    std::vector<int> images;
    int   image_index = 0;
    float anim_time = 0.0f;

    // �p�^�[���Ǘ�
    Enemy4Pattern pattern = Enemy4Pattern::Entrance;
    float pattern_timer = 0.0f;

    // �������o
    Vector2D enter_start = 0.0f;
    Vector2D target_pos = 0.0f;
    float    enter_time = 1.6f;   // 1.6�b�Ŏ���
    float    scale_from = 2.0f;   // ���Ȃ�傫��
    float    scale_to = 1.2f;   // �傫�߂��L�[�v
    float    scale_draw = 1.2f;   // ���݃X�P�[��
    int      alpha = 0;      // �t�F�[�h�C��

    // �o�g�����̜p�j
    Vector2D wander_target = 0.0f;
    float    wander_timer = 0.0f;
    float    next_wander = 1.2f;  // 1.0?1.8�b�ōX�V�i�ρj
    float    noise_t = 0.0f;

    // �U��
    float    shot_timer = 0.0f;
    int      attack_mode = 0;      // 0..3 �p�^�[��

    bool is_alive = false;

    // �ǉ��ipublic���j
    void OnHitCollision(GameObjectBase* hit_object) override;

    // �ǉ��iprivate�ł�OK�j
    float beam_damage_timer = 0.0f;

    
};
