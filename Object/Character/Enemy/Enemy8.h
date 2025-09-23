#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet2.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include <vector>

enum class Enemy8State {
    Appearing,
    Leaving
};

class Enemy8 : public EnemyBase {
public:
    Enemy8() = default;
    ~Enemy8() override = default;

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetAppearParams(const Vector2D& start, const Vector2D& end, float time);
    void SetPlayer(Player* p) { player = p; }

protected:
    void Shot(float delta_second);

private:
    // 見た目
    std::vector<int> images;
    int image_index = 0;
    float animation_time = 0.0f;
    float scale = 1.0f;      // DrawRotaGraph用（Enemy1と同じ流儀）
    int   alpha = 0;

    // 登場演出
    Enemy8State state = Enemy8State::Appearing;
    Vector2D start_location = 0.0f;
    Vector2D target_location = 0.0f;
    float appear_timer = 0.0f;
    float appear_duration = 1.2f;
    float scale_min = 0.3f;
    float scale_max = 1.2f;

    // 退場
    float leave_vy = 90.0f;

    // 射撃
    bool shot_fired = false;
    void ShootOnce();
};
