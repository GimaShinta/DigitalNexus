#include "Boss2.h"
#include "Boss2RotatingPart.h"
#include "../Player/Player.h"
#include "../../Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include"../../../Object/Beam/BeamEffects.h"
#include <cmath>

Boss2::Boss2()
{}
Boss2::~Boss2()
{}

void Boss2::Initialize()
{
    z_layer = 2;
    box_size = 40;
    hp = 4000;

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);

    location = { 640.0f, 240.0f };
    velocity = { 0.0f, 0.0f };

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy74_b.png", 6, 6, 1, 48, 48);

   /* if (images.empty())
    {
        images = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy75_b.png", 6, 6, 1, 40, 40);
    }*/

    image = images.empty() ? -1 : images[0];

    anim_indices.clear();
    for (int i = 0; i < (int)images.size(); i++)
    {
        anim_indices.push_back(i);
    }

    // === 6�̂̉�]�p�[�c���� ===
    const float radius = 250.0f;          // �������L����
    const float angle_step = 360.0f / 6;  // 60������
    for (int i = 0; i < 6; i++)
    {
        auto part = Singleton<GameObjectManager>::GetInstance()
            ->CreateObject<Boss2RotatingPart>(location);
        part->SetUp(this, radius, i * angle_step);
        rotating_parts.push_back(part);
    }

    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255, sound_destroy);
}

void Boss2::Update(float delta_second)
{
    if (hp <= 0)
    {
        is_alive = false;
        is_destroy = true;

        PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);

        auto* manager = Singleton<EffectManager>::GetInstance();
        int anim_id = manager->PlayerAnimation(EffectName::eExprotion, location, 0.05f, false);
        manager->SetScale(anim_id, 0.7f);

        Singleton<ScoreData>::GetInstance()->AddScore(1500);
        return;
    }

    if (!images.empty() && !anim_indices.empty())
    {
        GameObjectBase::AnimationControl(delta_second, images, anim_indices, 10.0f);
    }

    // �j�󂳂�Ă�����Đ����i�C�Ӂj
    if (rotating_parts.empty())
    {
        const float radius = 250.0f;
        const float angle_step = 360.0f / 6;
        for (int i = 0; i < 6; i++)
        {
            auto part = Singleton<GameObjectManager>::GetInstance()
                ->CreateObject<Boss2RotatingPart>(location);
            part->SetUp(this, radius, i * angle_step);
            rotating_parts.push_back(part);
        }
    }

    Shot(delta_second);
    __super::Update(delta_second);
}

void Boss2::Draw(const Vector2D& screen_offset) const
{
    if (image == -1) return;

    float scale = 4.5f;
    DrawRotaGraph(location.x, location.y, scale, 0.0f, image, TRUE);

    if (is_flashing)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
        DrawCircle((int)location.x, (int)location.y, 60, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

void Boss2::Finalize()
{
}

void Boss2::Shot(float delta_second)
{
    if (!player) return;

    if (hp > 4000 * 2 / 3)
    {
        shot_timer += delta_second;
        if (shot_timer >= 3.0f)
        {
            if (!pattern_toggle)  // �� �p�^�[��A�F�O�p�`�r�[������
            {
                int tri_indices[] = { 0, 2, 4 };
                for (int j = 0; j < 3; j++)
                {
                    int a = tri_indices[j];
                    int b = tri_indices[(j + 1) % 3];

                    if (rotating_parts[a] && rotating_parts[b])
                    {
                        auto tri_warn = Singleton<GameObjectManager>::GetInstance()->CreateObject<TriangleWarningBeam>(rotating_parts[a]->GetLocation());
                        tri_warn->SetEndpoints(rotating_parts[a], rotating_parts[b]);
                        tri_warn->SetLifeTime(0.6f);
                    }
                }
            }
            else  // �� �p�^�[��B�F���S or �O�����r�[���i�]���ʂ� group_toggle �g�p�j
            {
                if (!group_toggle)  // �����F���S��
                {
                    for (size_t i = 0; i < rotating_parts.size(); ++i)
                    {
                        if (!rotating_parts[i]) continue;
                        if (i % 2 == 0)
                        {
                            auto warn = Singleton<GameObjectManager>::GetInstance()->CreateObject<LinkedWarningBeam>(rotating_parts[i]->GetLocation());
                            warn->SetEndpoints(rotating_parts[i], this);
                            warn->SetLifeTime(0.6f);
                        }
                    }
                }
                else  // ��F�O����
                {
                    for (size_t i = 0; i < rotating_parts.size(); ++i)
                    {
                        if (!rotating_parts[i]) continue;
                        if (i % 2 == 1)
                        {
                            float deg = rotating_parts[i]->GetAngleGlobal();
                            float rad = deg * 3.1415926f / 180.0f;
                            Vector2D dir = Vector2D(cosf(rad), sinf(rad));

                            auto warn = Singleton<GameObjectManager>::GetInstance()->CreateObject<WarningBeam>(rotating_parts[i]->GetLocation());
                            warn->SetUp(dir, 600.0f, 0.6f);
                            warn->SetFollowTarget(rotating_parts[i]);
                        }
                    }
                }

                group_toggle = !group_toggle;  // �/�����r�[���ؑ�
            }

            pattern_toggle = !pattern_toggle;  // �p�^�[���ؑ�
            shot_timer = 0.0f;
        }
    }
}


bool Boss2::GetIsAlive() const
{
    return is_alive;
}
