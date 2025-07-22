#include "EnemyBeam.h"
#include "../GameObjectManager.h"
#include "../../Utility/ProjectConfig.h"
#include "../../Utility/EffectManager.h"
#include "../../Utility/InputManager.h"
#include "../Character/Boss/Boss3.h"
#include "../Character/Boss/Boss4.h"
#include <algorithm>

EnemyBeam::EnemyBeam()
{
}

EnemyBeam::~EnemyBeam()
{
}

// 初期化処理
void EnemyBeam::Initialize()
{
    z_layer = 2;
    box_size = Vector2D(8.0f, 800.0f);  // 最初は細く（予兆線用）

    collision.is_blocking = false;  // 最初は当たり判定なし
    collision.object_type = eObjectType::eEnemyBeam;
    collision.hit_object_type.push_back(eObjectType::ePlayer);

    is_mobility = true;

    beam_time = 0.0f;
    state_timer = 0.0f;
    state = BeamState::Warning;

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    beam_ts = rm->GetImages("Resource/Image/Object/Player/Beam/anime_sp_weapon03_1.png", 12, 2, 6, 88, 128);
    beam_t = beam_ts[10];
    beam_bs = rm->GetImages("Resource/Image/Object/Player/Beam/anime_sp_weapon03_2.png", 12, 2, 6, 88, 80);
    beam_b = beam_bs[10];


}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void EnemyBeam::Update(float delta_second)
{
#if 0
    beam_time += delta_second;
    alpha_timer += delta_second;

    // 太さの変化（細い → 太い）
    float max_thickness = 48.0f;
    float min_thickness = 8.0f;
    float thickness = min_thickness;

    if (beam_time < growth_duration)
    {
        float t = beam_time / growth_duration;
        thickness = min_thickness + (max_thickness - min_thickness) * t;
    }
    else
    {
        thickness = max_thickness;
    }

    box_size = Vector2D(thickness, box_size.y);

    // ビームの終了処理
    if (beam_time >= 5.0f)
    {
        SetDestroy();
        beam_time = 0.0f;
    }

    location += velocity * delta_second;


#else

    state_timer += delta_second;

    switch (state)
    {
    case BeamState::Warning:

        if (state_timer >= 0.5f)
        {
            if (boss4 != nullptr)
            {
                EffectManager* am = Singleton<EffectManager>::GetInstance();
                int anim = am->PlayerAnimation(EffectName::eChenge, Vector2D(location.x, location.y - box_size.y), 0.03f, false);
                am->SetScale(anim, 2.0f);
                am->SetAlpha(anim, 30);
                am->SetZLayer(anim, 2);

            }
            else if (boss3 != nullptr)
            {
                EffectManager* am = Singleton<EffectManager>::GetInstance();
                int anim = am->PlayerAnimation(EffectName::eChenge, Vector2D(location.x, location.y - box_size.y), 0.02f, false);
                am->SetScale(anim, 0.7f);
                am->SetAlpha(anim, 30);
                am->SetZLayer(anim, 2);

            }

        }
        if (state_timer >= warning_time)
        {
            state = BeamState::Firing;
            state_timer = 0.0f;
            beam_time = 0.0f;
            box_size.x = min_thickness;
            collision.hit_object_type.clear();
            collision.hit_object_type.push_back(eObjectType::ePlayer);
            SEManager::GetInstance()->PlaySE(SE_NAME::EnemyBeam);
        }
        else
        {
            collision.hit_object_type.clear();
            Singleton<ShakeManager>::GetInstance()->StartShake(1.5, 3, 3);
        }
        break;

    case BeamState::Firing:
        beam_time += delta_second;

        // 拡大処理
        if (beam_time < growth_duration_b)
        {
            float t = beam_time / growth_duration_b;
            box_size.x = min_thickness + (max_thickness - min_thickness) * t;
        }
        else
        {
            box_size.x = max_thickness;
            // 成長が終わったら Holding に
            state = BeamState::Holding;
            state_timer = 0.0f;
            Singleton<ShakeManager>::GetInstance()->StartShake(5.0, 5, 5);
        }

        location += velocity * delta_second;
        break;

    case BeamState::Holding:
        beam_time += delta_second; // ← これを追加！

        state_timer += delta_second;
        box_size.x = max_thickness;

        if (state_timer >= 4.0f)
        {
            state = BeamState::Shrinking;
            state_timer = 0.0f;
            beam_time = 0.0f; // ← ここでリセットしておけば縮小にちょうど使える
        }

        location += velocity * delta_second;
        break;

    case BeamState::Shrinking:
        beam_time += delta_second;

        if (beam_time < growth_duration_s)
        {
            float t = 1.0f - (beam_time / growth_duration_s);
            box_size.x = min_thickness + (max_thickness - min_thickness) * t;
        }
        else
        {
            box_size.x = min_thickness;
            SetDestroy(); // 終了
        }

        location += velocity * delta_second;
        break;
    }


    if (!beams_b.empty() && !beams_t.empty())
    {
        std::vector<int> animation_num = { 0, 1 };
        //フレームレートで時間を計測
        animation_time += delta_second;
        //8秒経ったら画像を切り替える
        if (animation_time >= 0.02f)
        {
            //計測時間の初期化
            animation_time = 0.0f;
            //時間経過カウントの増加
            animation_count++;
            //カウントがアニメーション画像の要素数以上になったら
            if (animation_count >= animation_num.size())
            {
                //カウントの初期化
                animation_count = 0;
            }
            // アニメーションが順番に代入される
            beam_t = beams_t[animation_num[animation_count]];
            beam_b = beams_b[animation_num[animation_count]];
        }
    }

    if (boss3 != nullptr && boss3->GetIsCrashing())
    {
        is_destroy = true;
    }
    if (boss4 != nullptr && boss4->GetIsCrashing())
    {
        is_destroy = true;
    }

#endif
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBeam::Draw(const Vector2D& screen_offset) const
{
#if 0
    // アルファ（透明度）をサイン波で変化 → 点滅
    float alpha = 255.0f;
    if (beam_time < growth_duration)
    {
        alpha = 128 + 127 * sinf(alpha_timer * 10.0f);  // 点滅周期は10Hz
    }

    int r = 255;
    int g = 0;
    int b = 255;

    int color = GetColor(r, g, b);
    int alpha_i = static_cast<int>(alpha);

    // 透過付き描画（DxLib）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha_i);

    DrawBox(
        static_cast<int>(location.x - box_size.x),
        static_cast<int>(location.y - box_size.y),
        static_cast<int>(location.x + box_size.x),
        static_cast<int>(location.y + box_size.y),
        color,
        TRUE
    );

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);  // 元に戻す
#else

    if (checked)
    {

        if (state == BeamState::Firing || state == BeamState::Shrinking || state == BeamState::Holding)
        {
            float t = 1.0f;

            if (state == BeamState::Firing)
            {
                t = (beam_time < growth_duration_b) ? beam_time / growth_duration_b : 1.0f;
            }
            else if (state == BeamState::Shrinking)
            {
                t = (beam_time < growth_duration_s) ? (1.0f - beam_time / growth_duration_s) : 0.0f;
            }
            else if (state == BeamState::Holding)
            {
                t = 1.0f; // 最大サイズ固定
            }

            float half_width = min_thickness_d + (max_thickness_d - min_thickness_d) * t;

            // 波揺れ + オフセット処理はそのままでOK
            float wave_amplitude = 2.0f;
            float wave_speed = 100.0f;
            float wave_offset = sinf(beam_time * wave_speed) * wave_amplitude;

            int y_offset = 80;

            int cx = static_cast<int>(location.x);
            int top_y = static_cast<int>(location.y - box_size.y + wave_offset + y_offset);
            int bottom_y = static_cast<int>(location.y + box_size.y + wave_offset + y_offset);

            DrawModiGraph(
                cx - half_width, top_y,
                cx + half_width, top_y,
                cx + half_width, bottom_y,
                cx - half_width, bottom_y,
                beam_t,
                TRUE
            );

            // 下側ビーム装飾（そのままでOK）
            int beam_b_height = 80;
            int base_bottom_y = static_cast<int>(location.y - box_size.y + wave_offset + 80);
            int base_top_y = base_bottom_y - beam_b_height;

            DrawModiGraph(
                cx - half_width, base_bottom_y,
                cx + half_width, base_bottom_y,
                cx + half_width, base_top_y,
                cx - half_width, base_top_y,
                beam_b,
                TRUE
            );
        }
        else if (state == BeamState::Warning)
        {
            // 赤い予兆線（半透明）
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
            DrawBox(
                static_cast<int>(location.x - box_size.x),
                static_cast<int>(location.y - box_size.y),
                static_cast<int>(location.x + box_size.x),
                static_cast<int>(location.y + box_size.y),
                GetColor(255, 0, 0),
                TRUE
            );
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
    }
    else
    {
        DrawBox(location.x - box_size.x, location.y - box_size.y,
            location.x + box_size.x, location.y + box_size.y,
            GetColor(255, 100, 0), TRUE);

    }

#endif

}

// 終了時処理
void EnemyBeam::Finalize()
{
}

void EnemyBeam::OnHitCollision(GameObjectBase* hit_object)
{
}

void EnemyBeam::SetBoss3(Boss3* p_boss)
{
    boss3 = p_boss;

    max_thickness = 48.0f;
    min_thickness = 4.0f;
    max_thickness_d = 48.0f;
    min_thickness_d = 4.0f;
    growth_duration_b = 1.0f; // 拡大時間
    growth_duration_s = 0.2f; // 縮小時間

    beams_t.push_back(beam_ts[9]);
    beams_t.push_back(beam_ts[10]);
    beams_b.push_back(beam_bs[9]);
    beams_b.push_back(beam_bs[10]);
}
void EnemyBeam::SetBoss4(Boss4* p_boss)
{
    boss4 = p_boss;

    max_thickness = 110.0f;
    min_thickness = 16.0f;
    max_thickness_d = 140.0f;
    min_thickness_d = 16.0f;
    growth_duration_b = 4.0f;
    growth_duration_s = 0.5f;

    beams_t.push_back(beam_ts[0]);
    beams_t.push_back(beam_ts[4]);
    beams_b.push_back(beam_bs[0]);
    beams_b.push_back(beam_bs[4]);

}
