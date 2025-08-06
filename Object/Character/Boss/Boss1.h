#pragma once
#include "../Enemy/EnemyBase.h"

enum class BossPattern
{
    Entrance,
    Hovering,
    Dead
};

class Boss1 : public EnemyBase
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

    void OnHitCollision(GameObjectBase* hit_object) override;


private:
    // �s���p�^�[���X�V
    void UpdateEntrance(float delta_second);
    void UpdateHovering(float delta_second);

    // �U���p�^�[��
    void ShotSpiral(float delta_second);
    void ShotAllRange();
    void ShotCrossShot();
    void ShotFanWide();  // ���e���n�p�^�[��
    void ShotWaveBullets();          // ���ǉ��p�^�[��
    void ShotTripleSpread();         // ���ǉ��p�^�[��
    void ShotFastSpiral(float delta_second); // ���ǉ��p�^�[��
    void ExplosionEffect(float delta_second);

private:
    BossPattern pattern = BossPattern::Entrance;
    bool is_alive = true;

    // �p�^�[���Ǘ�
    float pattern_timer = 0.0f;
    int attack_mode = 0;

    // �ړ��E�o�ꉉ�o
    Vector2D target_pos;
    Vector2D velocity;
    float move_timer = 0.0f;

    // �U���Ǘ�
    float shot_timer = 0.0f;

    // ���o
    float alpha = 0.0f;

    // �����Ǘ�
    bool explosions_started = false;
    float explosion_timer = 0.0f;
    int explosion_count = 0;
    bool final_explosion_done = false;

    // �摜�E��
    std::vector<int> images;
    int image = 0;
    int sound_destroy = -1;

    float aura_timer = 0.0f;
    float ripple_timer = 0.0f;

    float beam_damage_timer = 0.0f;
    const int BOSS1_MAX_HP = 8000;

   


};
