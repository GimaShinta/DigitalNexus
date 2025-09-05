#pragma once
#include "../GameObjectBase.h"

class OmegaBom : public GameObjectBase
{
private:
    class Player* player = nullptr;
    float expand_timer;
    float expand_duration;   // 1秒で最大サイズに到達
    float max_size;    // 最大サイズ
    float rotation_angle;
    float rotation_speed;
public:
    OmegaBom();
    ~OmegaBom();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;
    void OnHitCollision(GameObjectBase* hit_object) override;

    void SetPlayer(class Player* p) { player = p; }
};
