#pragma once
#include "../Enemy/EnemyBase.h"
#include "Boss2RotatingPart.h"

class Boss2 : public EnemyBase
{
public:
    Boss2();
    ~Boss2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    bool GetIsAlive() const;

    void SetPlayer(class Player* p)
    {
        player = p;
    }

private:
    void Shot(float delta_second);

    bool is_alive = true;
    float shot_timer = 0.0f;

    bool is_flashing = false;
    float flash_timer = 0.0f;
    bool visible = true;
    bool group_toggle = false; // ビームグループ切り替えフラグ

    std::vector<int> images;
    std::vector<int> images_a;
    std::vector<int> images_b;
    std::vector<int> anim_indices;
    std::vector<Boss2RotatingPart*> rotating_parts;

    int sound_destroy = NULL;
    class Player* player = nullptr;
    bool pattern_toggle = false;  // 新しく追加：攻撃パターン切替（true/falseで交互）
};
