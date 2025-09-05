#include "OmegaBom.h"
#include "../Character/Player/Player.h"
#include "../../Utility/EffectManager.h"
#include "DxLib.h"
#include <cmath>

OmegaBom::OmegaBom() :
    expand_timer(0.0f),
    expand_duration(4.0f),
    max_size(1000.0f),
    rotation_angle(0.0f),
    rotation_speed(900.0f) // ← 90度/秒で回転
{
}

OmegaBom::~OmegaBom() {}

void OmegaBom::Initialize()
{
    z_layer = 2;
    is_mobility = true;

    collision.is_blocking = true;
    collision.object_type = eObjectType::eDefenceShot;
    collision.hit_object_type.push_back(eObjectType::eEnemyShot);

    if (player) location = player->GetLocation();

    box_size = Vector2D(0.0f, 0.0f);

    expand_timer = 0.0f;
    rotation_angle = 0.0f;
}

// イージング関数（イーズアウト）
static float EaseOutQuad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

void OmegaBom::Update(float dt)
{
    if (player && player->GetRecoveryOn() == false) {
        is_destroy = true;
        return;
    }

    if (player) {
        location = player->GetLocation();
    }

    // 拡大アニメーション
    if (expand_timer < expand_duration) {
        expand_timer += dt;
        float t = expand_timer / expand_duration;
        if (t > 1.0f) t = 1.0f;

        float eased = EaseOutQuad(t);
        float size = max_size * eased;

        box_size = Vector2D(size, size);
    }

    // 回転
    rotation_angle += rotation_speed * dt;
    if (rotation_angle >= 360.0f) rotation_angle -= 360.0f;
}

void OmegaBom::Draw(const Vector2D&) const
{
    const int alpha = 80; // 半透明
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // 半サイズ（box_size は半径的に使っている前提）
    const float half_w = box_size.x;
    const float half_h = box_size.y;

    // 回転（ラジアン）
    const float rad = rotation_angle * DX_PI / 180.0f;
    const float c = cosf(rad);
    const float s = sinf(rad);

    // 中心原点のローカル頂点（回転前）
    Vector2D local[4];
    local[0] = Vector2D(-half_w, -half_h); // 左上
    local[1] = Vector2D(half_w, -half_h); // 右上
    local[2] = Vector2D(half_w, half_h); // 右下
    local[3] = Vector2D(-half_w, half_h); // 左下

    // 回転 → ワールド座標へ（location を中心に）
    Vector2D v[4];
    for (int i = 0; i < 4; ++i) {
        const float rx = local[i].x * c - local[i].y * s;
        const float ry = local[i].x * s + local[i].y * c;
        v[i] = Vector2D(location.x + rx, location.y + ry);
    }

    const unsigned int col = GetColor(200, 50, 50);

    // 2枚の三角形で塗りつぶし（AA版で境目が目立ちにくい）
    DrawTriangleAA(v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, col, TRUE);
    DrawTriangleAA(v[2].x, v[2].y, v[3].x, v[3].y, v[0].x, v[0].y, col, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
void OmegaBom::Finalize() {}

void OmegaBom::OnHitCollision(GameObjectBase* hit_object)
{
    if (!hit_object) return;

    EffectManager* em = Singleton<EffectManager>::GetInstance();
    int id = em->PlayerAnimation(EffectName::eExprotion2, hit_object->GetLocation(), 0.03f, false);
    em->SetAlpha(id, 180);
    em->SetScale(id, 0.5f);
    em->SetZLayer(id, z_layer + 1);
}
