#include "BeamEffects.h"
#include "DxLib.h"
#include "../../Object/GameObjectManager.h"

// ビーム太さ統一マクロ
#define BEAM_THICKNESS 36.0f
#define WARNING_LINE_THICKNESS 6
#define BEAM_LIFE_TIME 0.8f  // ★ ビームが消えるまでの時間（秒）

// ===================== FollowBeam =====================
void FollowBeam::Initialize()
{
    z_layer = 2;
    is_destroy = false;

    box_size = Vector2D(8.0f, 8.0f);
    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemyShot;
    collision.hit_object_type.push_back(eObjectType::ePlayer);
}

void FollowBeam::SetFollowTarget(GameObjectBase* target, const Vector2D& direction)
{
    follow_target = target;
    dir = direction;
}

void FollowBeam::SetBeamTarget(Vector2D target_position)
{
    target_pos = target_position;
    Vector2D delta = target_pos - location;
    length = delta.Length();
}

void FollowBeam::SetFollowAndLookTarget(GameObjectBase* follow, GameObjectBase* look, float max_length)
{
    follow_target = follow;
    look_target = look;
    length = max_length;
}

void FollowBeam::SetBeamThickness(float thickness)
{
    beam_thickness = thickness;
    initial_thickness = thickness;
}

void FollowBeam::SetBeamTextures(const std::vector<int>& tops, const std::vector<int>& bottoms)
{
    beam_textures_top = tops;
    beam_textures_bottom = bottoms;

    if (!beam_textures_top.empty()) beam_texture_top = beam_textures_top[0];
    if (!beam_textures_bottom.empty()) beam_texture_bottom = beam_textures_bottom[0];
}

void FollowBeam::Update(float delta_second)
{
    elapsed_time += delta_second;

    float t = elapsed_time / life_time;
    if (t > 1.0f) t = 1.0f;
    beam_thickness = (1.0f - t) * initial_thickness;

    if (elapsed_time >= life_time)
    {
        is_destroy = true;
        return;
    }

    if (follow_target)
    {
        location = follow_target->GetLocation();
    }

    if (look_target)
    {
        Vector2D to = look_target->GetLocation() - location;
        float len = to.Length();
        if (len > 0.01f)
        {
            dir = to / len;
            length = len;
        }
    }

    __super::Update(delta_second);

    constexpr int hitbox_count = 10;
    Vector2D beam_origin = location;

    for (int i = 0; i < hitbox_count; ++i)
    {
        float t = (i + 1) / static_cast<float>(hitbox_count + 1);
        Vector2D check_pos = beam_origin + dir * (length * t);

        BoxCollision temp_collision;
        temp_collision.point[0] = check_pos - Vector2D(6.0f, 6.0f);
        temp_collision.point[1] = check_pos + Vector2D(6.0f, 6.0f);
        temp_collision.object_type = collision.object_type;
        temp_collision.is_blocking = true;
        temp_collision.hit_object_type = collision.hit_object_type;

        for (GameObjectBase* obj : Singleton<GameObjectManager>::GetInstance()->GetGameObjects())
        {
            const auto& obj_col = obj->GetCollision();
            if (obj_col.object_type == eObjectType::ePlayer)
            {
                BoxCollision obj_copy = obj_col;
                obj_copy.point[0] += obj->GetLocation() - obj->GetBoxSize();
                obj_copy.point[1] += obj->GetLocation() + obj->GetBoxSize();

                if (IsCheckCollision(temp_collision, obj_copy))
                {
                    obj->OnHitCollision(this);
                    this->OnHitCollision(obj);
                    return;
                }
            }
        }
    }

    animation_time += delta_second;
    if (!beam_textures_top.empty() && animation_time >= 0.03f)
    {
        animation_time = 0.0f;
        animation_count = (animation_count + 1) % beam_textures_top.size();
        beam_texture_top = beam_textures_top[animation_count];
        beam_texture_bottom = beam_textures_bottom[animation_count];
    }
}

void FollowBeam::Draw(const Vector2D& screen_offset) const
{
    if (beam_texture_top < 0 || beam_texture_bottom < 0)
    {
        Vector2D start = location + screen_offset;
        Vector2D end = start + dir * length;

        SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
        DrawLine(start.x, start.y, end.x, end.y, GetColor(100, 255, 255), static_cast<int>(beam_thickness));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        DrawCircle(static_cast<int>(location.x), static_cast<int>(location.y), static_cast<int>(box_size.x), GetColor(255, 255, 0), FALSE);
        return;
    }

    Vector2D start = location + screen_offset;
    Vector2D end = start + dir * length;

    float wave_amplitude = 2.0f;
    float wave_speed = 100.0f;
    float wave_offset = sinf(elapsed_time * wave_speed) * wave_amplitude;

    Vector2D normal = Vector2D(-dir.y, dir.x);

    Vector2D top1 = start + normal * (beam_thickness + wave_offset);
    Vector2D top2 = end + normal * (beam_thickness + wave_offset);
    Vector2D bottom2 = end - normal * (beam_thickness + wave_offset);
    Vector2D bottom1 = start - normal * (beam_thickness + wave_offset);

    SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
    DrawModiGraph(top1.x, top1.y, top2.x, top2.y, bottom2.x, bottom2.y, bottom1.x, bottom1.y, beam_texture_top, TRUE);

    Vector2D deco1 = start + normal * (beam_thickness + 25.0f);
    Vector2D deco2 = end + normal * (beam_thickness + 25.0f);
    Vector2D deco3 = end + normal * (beam_thickness + 40.0f);
    Vector2D deco4 = start + normal * (beam_thickness + 40.0f);

    DrawModiGraph(deco1.x, deco1.y, deco2.x, deco2.y, deco3.x, deco3.y, deco4.x, deco4.y, beam_texture_bottom, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void FollowBeam::OnHitCollision(GameObjectBase* hit_object)
{}

// ===================== LinkedBeam =====================
void LinkedBeam::Initialize()
{
    z_layer = 2;
    is_destroy = false;

    // ▼ 当たり判定の基本設定（敵弾→プレイヤーに当てる）
    box_size = Vector2D(8.0f, 8.0f);
    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemyShot;
    collision.hit_object_type.clear();
    collision.hit_object_type.push_back(eObjectType::ePlayer);
}

void LinkedBeam::SetEndpoints(GameObjectBase* a, GameObjectBase* b)
{
    part_a = a;
    part_b = b;
}

void LinkedBeam::SetThickness(float thickness)
{
    beam_thickness = thickness;
    initial_thickness = thickness; // ★ 太さの初期値を保存
}

void LinkedBeam::SetLifeTime(float time)
{
    life_time = time;
}

void LinkedBeam::Update(float delta_second)
{
    elapsed_time += delta_second;

    // 時間に応じて太さを細くする
    float t = elapsed_time / life_time;
    if (t > 1.0f) t = 1.0f;
    beam_thickness = (1.0f - t) * initial_thickness;

    if (elapsed_time >= life_time)
    {
        is_destroy = true;
    }

    __super::Update(delta_second);

    // ▼ 当たり判定（線分上に複数の点判定を打つ）
    if (part_a && part_b && !is_destroy)
    {
        Vector2D start = part_a->GetLocation();
        Vector2D end = part_b->GetLocation();
        Vector2D seg = end - start;
        float len = seg.Length();

        if (len > 0.01f)
        {
            Vector2D dir = seg / len;

            constexpr int hitbox_count = 10;  // 必要なら長さに応じて増やしてもOK
            for (int i = 0; i < hitbox_count; ++i)
            {
                float tt = (i + 1) / static_cast<float>(hitbox_count + 1);
                Vector2D check_pos = start + dir * (len * tt);

                BoxCollision temp_collision;
                // FollowBeamと同じく 12x12 程度の小BOXで判定
                const float half = 6.0f;
                temp_collision.point[0] = check_pos - Vector2D(half, half);
                temp_collision.point[1] = check_pos + Vector2D(half, half);
                temp_collision.object_type = collision.object_type;
                temp_collision.is_blocking = true;
                temp_collision.hit_object_type = collision.hit_object_type;

                for (GameObjectBase* obj : Singleton<GameObjectManager>::GetInstance()->GetGameObjects())
                {
                    const auto& obj_col = obj->GetCollision();
                    if (obj_col.object_type == eObjectType::ePlayer)
                    {
                        BoxCollision obj_copy = obj_col;
                        obj_copy.point[0] += obj->GetLocation() - obj->GetBoxSize();
                        obj_copy.point[1] += obj->GetLocation() + obj->GetBoxSize();

                        if (IsCheckCollision(temp_collision, obj_copy))
                        {
                            obj->OnHitCollision(this);
                            this->OnHitCollision(obj);
                            return; // 多重ヒットを避けて終了
                        }
                    }
                }
            }
        }
    }
}

void LinkedBeam::Draw(const Vector2D& screen_offset) const
{
    if (part_a && part_b)
    {
        Vector2D start = part_a->GetLocation() + screen_offset;
        Vector2D end = part_b->GetLocation() + screen_offset;

        SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
        DrawLine(start.x, start.y, end.x, end.y, GetColor(255, 255, 0), static_cast<int>(beam_thickness));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}


// ===================== LinkedWarningBeam =====================
void LinkedWarningBeam::Initialize()
{
    z_layer = 2;
    is_destroy = false;
}

void LinkedWarningBeam::SetEndpoints(GameObjectBase* a, GameObjectBase* b)
{
    part_a = a;
    part_b = b;
}

void LinkedWarningBeam::SetLifeTime(float time)
{
    warning_time = time;
}

void LinkedWarningBeam::Update(float delta_second)
{
    elapsed_time += delta_second;
    if (elapsed_time >= warning_time)
    {
        if (part_a && part_b)
        {
            auto beam = Singleton<GameObjectManager>::GetInstance()->CreateObject<FollowBeam>(part_a->GetLocation());
            beam->SetFollowAndLookTarget(part_a, part_b, 600.0f);
            beam->SetBeamThickness(BEAM_THICKNESS);
            beam->SetLifeTime(BEAM_LIFE_TIME);  // ★ 寿命設定
        }
        is_destroy = true;
    }

    __super::Update(delta_second);
}

void LinkedWarningBeam::Draw(const Vector2D& screen_offset) const
{
    if (part_a && part_b)
    {
        Vector2D start = part_a->GetLocation() + screen_offset;
        Vector2D end = part_b->GetLocation() + screen_offset;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawLine(start.x, start.y, end.x, end.y, GetColor(255, 0, 0), WARNING_LINE_THICKNESS);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

// ===================== WarningBeam =====================
void WarningBeam::Initialize()
{
    z_layer = 2;
    is_destroy = false;
}

void WarningBeam::SetUp(const Vector2D& direction, float length, float lifetime)
{
    dir = direction.Normalize();
    beam_length = length;
    life_time = lifetime;
}

void WarningBeam::SetFollowTarget(GameObjectBase* target)
{
    follow_target = target;
}

void WarningBeam::SetFollowAndLookTarget(GameObjectBase* follow, GameObjectBase* look)
{
    follow_target = follow;
    look_target = look;
}

void WarningBeam::Update(float delta_second)
{
    if (follow_target)
    {
        location = follow_target->GetLocation();
    }

    if (look_target)
    {
        Vector2D to = look_target->GetLocation() - location;
        float len = to.Length();
        if (len > 0.01f)
        {
            dir = to / len;
            beam_length = len;
        }
    }

    elapsed_time += delta_second;
    if (elapsed_time >= life_time)
    {
        auto beam = Singleton<GameObjectManager>::GetInstance()->CreateObject<FollowBeam>(location);
        beam->SetFollowAndLookTarget(follow_target, look_target, beam_length);
        beam->SetFollowTarget(follow_target, dir);
        beam->SetBeamThickness(BEAM_THICKNESS);
        beam->SetLifeTime(BEAM_LIFE_TIME);  // ★ 寿命設定

        SetDestroy();
    }

    __super::Update(delta_second);
}

void WarningBeam::Draw(const Vector2D& screen_offset) const
{
    Vector2D start = location + screen_offset;
    Vector2D end = start + dir * beam_length;

    SetDrawBlendMode(DX_BLENDMODE_ADD, 128);
    DrawLine(start.x, start.y, end.x, end.y, GetColor(255, 0, 0), WARNING_LINE_THICKNESS);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// ===================== TriangleWarningBeam =====================
void TriangleWarningBeam::Initialize()
{
    z_layer = 2;
    is_destroy = false;
}

void TriangleWarningBeam::SetEndpoints(GameObjectBase* a, GameObjectBase* b)
{
    part_a = a;
    part_b = b;
}

void TriangleWarningBeam::SetLifeTime(float time)
{
    warning_time = time;
}

void TriangleWarningBeam::Update(float delta_second)
{
    elapsed_time += delta_second;
    if (elapsed_time >= warning_time)
    {
        if (part_a && part_b)
        {
            auto beam = Singleton<GameObjectManager>::GetInstance()->CreateObject<LinkedBeam>(part_a->GetLocation());
            beam->SetEndpoints(part_a, part_b);
            beam->SetThickness(BEAM_THICKNESS);
            beam->SetLifeTime(0.8f);
         }
        is_destroy = true;
    }

    __super::Update(delta_second);
}

void TriangleWarningBeam::Draw(const Vector2D& screen_offset) const
{
    if (part_a && part_b)
    {
        Vector2D start = part_a->GetLocation() + screen_offset;
        Vector2D end = part_b->GetLocation() + screen_offset;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawLine(start.x, start.y, end.x, end.y, GetColor(255, 128, 0), WARNING_LINE_THICKNESS);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}
