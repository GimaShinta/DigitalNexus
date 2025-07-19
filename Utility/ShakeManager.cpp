#include "ShakeManager.h"
#include <cstdlib> 

// �h�炵���J�n������
void ShakeManager::StartShake(float durationSec, float powerX, float powerY)
{
    duration = durationSec;
    timer = durationSec;
    power_x = powerX;
    power_y = powerY;
}

// �X�V����
void ShakeManager::Update(float deltaTime)
{
    if (timer > 0.0f)
    {
        timer -= deltaTime;
        if (timer < 0.0f)
            timer = 0.0f;

        float progress = 1.0f - (timer / duration);
        float decay = 1.0f - progress;

        float dx = ((rand() % 100 / 100.0f) * 2.0f - 1.0f) * power_x * decay;
        float dy = ((rand() % 100 / 100.0f) * 2.0f - 1.0f) * power_y * decay;

        offset = Vector2D(dx, dy);
    }
    else
    {
        offset = Vector2D(0.0f, 0.0f);
    }
}

// �h��I�t�Z�b�g�̎擾
Vector2D ShakeManager::GetOffset() const
{
    return offset;
}

void ShakeManager::Reset()
{
    timer = 0.0f;
    offset = Vector2D(0.0f, 0.0f);
}