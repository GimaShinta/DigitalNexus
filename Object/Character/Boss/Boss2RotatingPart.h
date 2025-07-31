#pragma once
#include "../../../Object/GameObjectBase.h"
class Boss2;

class Boss2RotatingPart : public GameObjectBase
{
public:
    Boss2RotatingPart();
    ~Boss2RotatingPart() override = default;

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void SetUp(Boss2* boss, float radius, float angle_offset);
    float GetAngleGlobal() const;

private:
    Boss2* boss = nullptr;
    float radius = 100.0f;       // ��]���a
    float angle = 0.0f;          // ���݊p�x
    float angle_speed = 50.0f;   // ��]���x�i�x/�b�j
    float angle_offset = 0.0f;   // �����ʑ�
    int image = -1;
};
