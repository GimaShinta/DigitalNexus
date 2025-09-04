#pragma once
#include "../GameObjectBase.h"

class OmegaBom : public GameObjectBase
{
private:
    class Player* player = nullptr;

    float radius = 300.0f;   // ç≈èâÇ©ÇÁå≈íËÇÃîºåa
    int alpha = 80;          // îñÇ≥Åi0?255Åj

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
