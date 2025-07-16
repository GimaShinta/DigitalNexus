#pragma once
#include "../Singleton.h"
#include "Vector2D.h"

class ShakeManager :
    public Singleton<ShakeManager>
{
private:
    float timer = 0.0f;
    float duration = 0.0f;
    float power_x = 0.0f;
    float power_y = 0.0f;
    Vector2D offset = { 0, 0 };

public:
    void StartShake(float duration, float powerX, float powerY);
    void Update(float deltaTime);
    Vector2D GetOffset() const;

    bool IsShaking() const { return timer > 0.0f; }
    void Reset();
};

