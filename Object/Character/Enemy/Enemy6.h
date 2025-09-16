#pragma once
#include "EnemyBase.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet3.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet5.h"

enum class Enemy6State { Appearing, Falling, Leaving };

class Enemy6 : public EnemyBase {
public:
    Enemy6();
    Enemy6(const Vector2D& pos);
    ~Enemy6();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPlayer(Player* p) { player = p; }

    // Enemy1風：上からフェードイン→そのまま降下
    void SetAppearParams(const Vector2D& start, float appear_time, float fall_speed);

protected:
    void Shot(float delta_second);
private:
    Enemy6State state = Enemy6State::Appearing;

    // 出現
    Vector2D start_pos = 0.0f;
    float appear_duration = 1.0f;
    float appear_timer = 0.0f;
    float scale_min = 1.0f, scale_max = 2.0f, scale = 1.0f;
    int   alpha = 0;

    // 降下
    float vy = 100.0f;

    // アニメ
    std::vector<int> images;
    int image = -1;
    float animation_time = 0.0f;
    int animation_index = 0;
};
