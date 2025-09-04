#include "OmegaBom.h"
#include "../Character/Player/Player.h"
#include "../../Utility/EffectManager.h"

OmegaBom::OmegaBom() {}
OmegaBom::~OmegaBom() {}

void OmegaBom::Initialize()
{
    z_layer = 3;
    is_mobility = true;

    // 防御ショット：敵弾を対象
    collision.is_blocking = true;
    collision.object_type = eObjectType::eDefenceShot;
    collision.hit_object_type.push_back(eObjectType::eEnemyShot);
    collision.hit_object_type.push_back(eObjectType::eEnemyBeam);

    if (player) location = player->GetLocation();

    // 判定も半径に合わせて設定
    box_size = Vector2D(radius * 2.0f, radius * 2.0f);
}

void OmegaBom::Update(float dt)
{
    if (player && player->GetRecoveryOn() == false) {
        is_destroy = true;
        return;
    }

    // プレイヤーに追従
    if (player) {
        location = player->GetLocation();
    }

    // box_size を毎フレーム radius に合わせて代入
    box_size = Vector2D(radius * 2.0f, radius * 2.0f);
}

void OmegaBom::Draw(const Vector2D&) const
{
    // ★塗りつぶし円（薄い半透明）
    int c = GetColor(160, 232, 255);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawCircle((int)location.x, (int)location.y, (int)radius, c, TRUE); // TRUE=塗りつぶし
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

#if _DEBUG
    int l = (int)(location.x - box_size.x * 0.5f);
    int t = (int)(location.y - box_size.y * 0.5f);
    int r = (int)(location.x + box_size.x * 0.5f);
    int b = (int)(location.y + box_size.y * 0.5f);
    DrawBox(l, t, r, b, GetColor(255, 0, 0), FALSE); // ←これが本当の当たり判定
#endif

}

void OmegaBom::Finalize() {}

void OmegaBom::OnHitCollision(GameObjectBase* hit_object)
{
    if (!hit_object) return;

    // 敵弾消滅演出
    EffectManager* em = Singleton<EffectManager>::GetInstance();
    int id = em->PlayerAnimation(EffectName::eExprotion2,
        hit_object->GetLocation(),
        0.03f, false);
    em->SetAlpha(id, 180);
    em->SetScale(id, 0.5f);
    em->SetZLayer(id, z_layer + 1);
}
