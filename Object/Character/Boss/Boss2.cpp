// Boss2.cpp
#include "Boss2.h"
#include "Boss2RotatingPart.h"
#include "../Player/Player.h"
#include "../../Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../../Object/Beam/BeamEffects.h"
#include <cmath>

namespace
{
    inline float EaseOutCubic(float t)
    { // 0→1
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return 1.0f - std::pow(1.0f - t, 3.0f);
    }
}

Boss2::Boss2()
{}
Boss2::~Boss2()
{}

void Boss2::Initialize()
{
    z_layer = 2;
    box_size = Vector2D(80, 80);
    hp = BOSS2_MAX_HP;

    // ★ イントロ中は当たり判定オフ（被弾で爆発しない）
    collision.is_blocking = false;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);

    location = { 640.0f, 240.0f };
    velocity = { 0.0f, 0.0f };

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy74_b.png", 6, 6, 1, 48, 48);
    image = images.empty() ? -1 : images[0];

    anim_indices.clear();
    for (int i = 0; i < (int)images.size(); ++i) anim_indices.push_back(i);

    // 回転パーツはここでは作らない（逐次スポーン）
    rotating_parts.clear();

    // 共有角・半径・スピード
    orbit_base_angle = 0.0f;
    orbit_speed_deg = 90.0f;
    orbit_radius = 250.0f;

    // イントロ初期化
    intro_active = true;
    intro_timer = 0.0f;
    appear_scale = 1.0f;
    intro_spawned_count = 0;
    part_spawn_timer = 0.0f;
    part_spawn_interval = 0.35f;

    // 破壊音は既存のまま
    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255, sound_destroy);

    // ★ 持続ダメージ系の初期化（Boss1相当）
    battle_started = false;
    damage_timer = 0.0f;
    beam_damage_timer = 0.0f;
}

void Boss2::Update(float delta_second)
{
    // 全パーツの共通回転角を進める
    orbit_base_angle += orbit_speed_deg * delta_second;
    if (orbit_base_angle >= 360.0f) orbit_base_angle -= 360.0f;
    // Boss2::Update(float delta_second) の先頭あたり
    cross_angle += delta_second * 0.5f;  // ゆっくり回転

    if (hp <= 0)
    {
        is_alive = false;
        is_destroy = true;

        // 回転パーツ破棄
        for (auto* part : rotating_parts)
            if (part) part->SetDestroy();
        rotating_parts.clear();

        if (sound_destroy >= 0) PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);

        auto* manager = Singleton<EffectManager>::GetInstance();
        int anim_id = manager->PlayerAnimation(EffectName::eExprotion, location, 0.05f, false);
        manager->SetScale(anim_id, 0.7f);

        Singleton<ScoreData>::GetInstance()->AddScore(1500);
        return;
    }

    // アニメーション
    //if (!images.empty() && !anim_indices.empty())
    //{
    //    GameObjectBase::AnimationControl(delta_second, images, anim_indices, 10.0f);
    //}

    int frame = static_cast<int>((GetNowCount() / 10) % images.size());
    image = images[frame];

    // ==== イントロ：ズーム＋1体ずつスポーン（爆発エフェクト無し） ====
    if (intro_active)
    {
        intro_timer += delta_second;
        part_spawn_timer += delta_second;

        // ズーム（easeOutCubic）
        const float zoom_dur = 2.2f;
        float t = EaseOutCubic(intro_timer / zoom_dur);
        appear_scale = 1.3f + (4.5f - 1.3f) * t;

        // 一定間隔で1体ずつスポーン
        const int total_target = 6;
        if (intro_spawned_count < total_target && part_spawn_timer >= part_spawn_interval)
        {
            part_spawn_timer = 0.0f;

            auto part = Singleton<GameObjectManager>::GetInstance()
                ->CreateObject<Boss2RotatingPart>(location);
            part->SetPlayer(player);
            part->SetUp(this, orbit_radius, 0.0f); // 角度オフセットは使わない
            part->SetIsBulletPart(intro_spawned_count % 2 == 1);
            rotating_parts.push_back(part);

            // 等間隔再配置（N が変わったので再インデックス）
            ReindexRing();

            // ※ 登場中は爆発エフェクトを発生させない

            ++intro_spawned_count;
        }

        // すべて出してズームも完了したら通常モードへ
        if (intro_spawned_count >= total_target && intro_timer >= (zoom_dur + 0.4f))
        {
            intro_active = false;

            // ※ 仕上げの爆発エフェクトも発生させない

            // ★ ここで当たり判定オン（ここから被弾OK）
            collision.is_blocking = true;
            battle_started = true;
            damage_timer = 0.0f;
            beam_damage_timer = 0.0f;
        }

        __super::Update(delta_second);
        return;
    }
    // ==============================================

    // 全滅→再生成（演出外の保険）
    if (rotating_parts.empty())
    {
        for (int i = 0; i < 6; ++i)
        {
            auto part = Singleton<GameObjectManager>::GetInstance()
                ->CreateObject<Boss2RotatingPart>(location);
            part->SetUp(this, orbit_radius, 0.0f);
            part->SetIsBulletPart(i % 2 == 1);
            rotating_parts.push_back(part);
        }
        ReindexRing();
    }


    // === Boss1同等：戦闘中の「持続ダメージ」 ===
    if (battle_started)
    {
        damage_timer += delta_second;
        if (damage_timer >= 0.05f)   // 0.05秒ごとに
        {
            damage_timer = 0.0f;
            hp -= 4.0f;              // HPを減少
        }
    }
    // Boss2.cpp Update内
    rotation_angle += delta_second * 0.8f;   // ゆっくり回転
    float_timer += delta_second;             // 浮遊アニメ用

    // 通常攻撃
    Shot(delta_second);
    __super::Update(delta_second);
}

void Boss2::Draw(const Vector2D& /*screen_offset*/) const
{
    if (image == -1) return;

    // 登場スケールを使用
    const float scale = appear_scale;
    // Boss2.cpp Draw内
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);

    float float_offset = std::sinf(float_timer * 1.5f) * 20.0f; // 上下に±10px揺れる
    DrawRotaGraph((int)location.x, (int)(location.y + float_offset),
        scale, rotation_angle, image, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // ★ イントロ中は静かな発光＋（任意で）演出強化
    if (intro_active)
    {
        // ベースのオーラ
        float pulse = 0.5f + 0.5f * sinf(GetNowCount() * 0.02f); // 0?1
        int alpha = (int)(120 + 80 * pulse); // 120?200
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        int r = (int)(60 + 20 * pulse);
        DrawCircle((int)location.x, (int)location.y, 50 + r, GetColor(180, 220, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 追加：淡いリング数本
        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
        for (int i = 0; i < 3; ++i)
        {
            float t = (GetNowCount() * 0.001f + i * 0.33f);
            float rr = 80.0f + fmodf(t, 1.0f) * 140.0f; // 80→220
            DrawCircle((int)location.x, (int)location.y, (int)rr, GetColor(200, 240, 255), FALSE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // 追加：稲妻っぽいノイズ線
        SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
        for (int i = 0; i < 5; ++i)
        {
            int len = 40 + GetRand(120);
            float ang = (float)GetRand(628) / 100.0f; // 0?6.28
            int x1 = (int)location.x;
            int y1 = (int)location.y;
            int x2 = x1 + (int)(cosf(ang) * len);
            int y2 = y1 + (int)(sinf(ang) * len);
            DrawLine(x1, y1, x2, y2, GetColor(200, 220, 255));
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    if (is_flashing)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
        DrawCircle((int)location.x, (int)location.y, 60, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // ★ HPバーはイントロ中は非表示
    if (!intro_active)
    {
        float hp_ratio = (float)hp / (float)BOSS2_MAX_HP;
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;
        if (hp_ratio > 1.0f) hp_ratio = 1.0f;

        const int bar_width = 640;
        const int bar_height = 6;
        const int bar_x = (D_WIN_MAX_X - bar_width) / 2;
        const int bar_y = 20;

        float wave_offset = sinf(GetNowCount() * 0.01f) * 1.0f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
        DrawBox(bar_x, bar_y + wave_offset, bar_x + bar_width, bar_y + bar_height + wave_offset, GetColor(50, 50, 50), TRUE);
        DrawBox(bar_x, bar_y + wave_offset, bar_x + (int)(bar_width * hp_ratio), bar_y + bar_height + wave_offset, GetColor(240, 80, 80), TRUE);
        DrawBox(bar_x, bar_y + wave_offset, bar_x + bar_width, bar_y + bar_height + wave_offset, GetColor(180, 180, 180), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Boss2::Finalize()
{}

void Boss2::Shot(float delta_second)
{
    if (!player) return;
    if (intro_active) return; // イントロ中は撃たない

    if (hp > BOSS2_MAX_HP * 2 / 3)
    {
        // ── 既存：三角形ビーム／中心 or 外向きビーム ─────────────────────
        shot_timer += delta_second;
        if (shot_timer >= 3.0f)
        {
            if (!pattern_toggle) // ▼ パターンA：三角形ビーム
            {
                int tri_indices[] = { 0, 2, 4 };
                for (int j = 0; j < 3; ++j)
                {
                    int a = tri_indices[j];
                    int b = tri_indices[(j + 1) % 3];
                    if (a < (int)rotating_parts.size() && b < (int)rotating_parts.size()
                        && rotating_parts[a] && rotating_parts[b])
                    {
                        auto tri_warn = Singleton<GameObjectManager>::GetInstance()
                            ->CreateObject<TriangleWarningBeam>(rotating_parts[a]->GetLocation());
                        tri_warn->SetEndpoints(rotating_parts[a], rotating_parts[b]);
                        tri_warn->SetLifeTime(0.6f);
                    }
                }
            }
            else // ▼ パターンB：中心 or 外向きビーム
            {
                if (!group_toggle)  // 偶数：中心へ
                {
                    for (size_t i = 0; i < rotating_parts.size(); ++i)
                    {
                        if (!rotating_parts[i]) continue;
                        if (i % 2 == 0)
                        {
                            auto warn = Singleton<GameObjectManager>::GetInstance()
                                ->CreateObject<LinkedWarningBeam>(rotating_parts[i]->GetLocation());
                            warn->SetEndpoints(rotating_parts[i], this);
                            warn->SetLifeTime(0.6f);
                        }
                    }
                }
                else  // 奇数：外向き
                {
                    for (size_t i = 0; i < rotating_parts.size(); ++i)
                    {
                        if (!rotating_parts[i]) continue;
                        if (i % 2 == 1)
                        {
                            float deg = rotating_parts[i]->GetAngleGlobal();
                            float rad = deg * 3.1415926535f / 180.0f;
                            Vector2D dir(std::cos(rad), std::sin(rad));

                            auto warn = Singleton<GameObjectManager>::GetInstance()
                                ->CreateObject<WarningBeam>(rotating_parts[i]->GetLocation());
                            warn->SetUp(dir, 600.0f, 0.6f);
                            warn->SetFollowTarget(rotating_parts[i]);
                        }
                    }
                }
                group_toggle = !group_toggle;
            }

            pattern_toggle = !pattern_toggle;
            shot_timer = 0.0f;
        }
    }
    //else if (hp > BOSS2_MAX_HP / 3)
    //{
    //    // ── 新規：クロスレーザー（上下左右4方向に警告→本ビーム） ─────────────
    //    // WarningBeam::SetUp(direction, length, lifetime) を使用:contentReference[oaicite:3]{index=3}
    //    shot_timer += delta_second;
    //    if (shot_timer >= 4.0f)
    //    {
    //        const int beams = 8;
    //        for (int i = 0; i < beams; ++i)
    //        {
    //            float ang = cross_angle + (DX_PI * 2.0f / beams) * i; // 360°/8
    //            Vector2D dir(std::cos(ang), std::sin(ang));

    //            auto warn = Singleton<GameObjectManager>::GetInstance()
    //                ->CreateObject<WarningBeam>(location);
    //            // 長さ: 900px, 警告時間: 0.8秒（警告後に FollowBeam が自動生成される）:contentReference[oaicite:4]{index=4}
    //            warn->SetUp(dir, 900.0f, 0.8f);
    //        }
    //        shot_timer = 0.0f;
    //    }
    //}
    //else
    //{
    //    // ── 低HP帯（必要ならここに“終盤専用”攻撃を追加可能） ────────────────
    //    // 例：クロスの間隔を短縮したい場合は 2.0f など
    //    shot_timer += delta_second;
    //    if (shot_timer >= 2.5f)
    //    {
    //        const int beams = 4;
    //        for (int i = 0; i < beams; ++i)
    //        {
    //            float ang = (3.1415926535f / 2.0f) * i;
    //            Vector2D dir(std::cos(ang), std::sin(ang));
    //            auto warn = Singleton<GameObjectManager>::GetInstance()
    //                ->CreateObject<WarningBeam>(location);
    //            warn->SetUp(dir, 900.0f, 0.6f); // 終盤は警告短め
    //        }
    //        shot_timer = 0.0f;
    //    }
    //}
}


bool Boss2::GetIsAlive() const
{
    return is_alive;
}

void Boss2::ReindexRing()
{
    const int n = (int)rotating_parts.size();
    if (n <= 0) return;

    for (int i = 0; i < n; ++i)
    {
        if (rotating_parts[i])
        {
            rotating_parts[i]->SetRingParams(i, n);
        }
    }
}


void Boss2::OnHitCollision(GameObjectBase* hit_object)
{
    auto type = hit_object->GetCollision().object_type;

    if (type == eObjectType::eAttackShot)
    {
        hp -= 10; // Boss1 と同値
    }

    if (type == eObjectType::eBeam)
    {
        // Boss1 は 1/60f を足し込む方式。毎フレーム衝突が来る前提で間引き。
        beam_damage_timer += 1.0f / 60.0f;
        if (beam_damage_timer >= 0.15f)
        {
            hp -= 5;             // Boss1 と同値
            beam_damage_timer = 0.0f;
        }
    }
}