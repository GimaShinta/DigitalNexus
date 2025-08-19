#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Enemy/Enemy4.h"
#include "../../../../Object/Character/Boss/Boss1.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage1::Stage1(Player* player)
	: StageBase(player)
{
}

Stage1::~Stage1()
{
}

// ����������
void Stage1::Initialize()
{
    // �X�e�[�WID�̐ݒ�
	stage_id = StageID::Stage1;

    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 48, 6, DX_FONTTYPE_ANTIALIASING);

    if (player)
    {
        entry_start_pos = Vector2D(D_WIN_MAX_X / 2, 360);             // ���������肩��
        entry_end_pos = Vector2D(D_WIN_MAX_X / 2, D_WIN_MAX_Y - 120); // �ʏ�̒�ʒu

        player_entry_timer = 0.0f; // �^�C�}�[���Z�b�g

        player->SetMobility(false); // ����֎~
        player->SetLocation(entry_start_pos); // �ŏ��̈ʒu��ݒ�
        player->ForceNeutralAnim(true); // �� �A�j�����Œ�
        player->SetShotStop(false);
    }

    // === �ǉ�: Enemy2 �X�P�W���[�������� ===
    e2_line_enabled = false;
    e2_group_next = 10.2f;  // Wave2 �J�n���ォ�班���x�点��
    e2_group_id = 0;

    e2_single_next = 11.2f;  // ���Ԃ̒P���͂���ɏ����x�点��
    e2_group_interval = 3.5f;
    e2_single_interval = 1.6f;

}

// �I��������
void Stage1::Finalize()
{
    if (font_orbitron != -1) {
        DeleteFontToHandle(font_orbitron);
        font_orbitron = -1;
    }
    if (font_warning != -1) {
        DeleteFontToHandle(font_warning);
        font_warning = -1;
    }
}

// �X�V����
void Stage1::Update(float delta_second)
{
    // �I�u�W�F�N�g�ƃG�t�F�N�g�̍X�V����
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
	objm->Update(delta_second);
	EffectManager* manager = Singleton<EffectManager>::GetInstance();
	manager->Update(delta_second);

    // �X�e�[�W�Ǘ��p�^�C�}�[
    stage_timer += delta_second;

    // �G�̏o��
    EnemyAppearance(delta_second);

    // �N���A���菈��
    UpdateGameStatus(delta_second);

    // �X�N���[���̍X�V����
	UpdateBackgroundScroll(delta_second);

    // �v���C���[�̓o��V�[��
    AppearancePlayer(delta_second);

    // �o�ꎞ�̃X�e�[�W���x���̍X�V����
    UpdateRabel(delta_second);
}

// �`�揈��
void Stage1::Draw()
{
    // �w�i�X�N���[���̕`��
    DrawScrollBackground();  

    // �I�u�W�F�N�g�̕`�揈��
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    objm->Draw();

    // �G�t�F�N�g�̕`�揈��
    EffectManager* em = Singleton<EffectManager>::GetInstance();
    em->Draw();

    // -------- �X�e�[�W���o�FNeural Grid --------
    StageLabel();

    //// �Q�[���I�[�o�[���̉��o
    //if (is_over)
    //{
    //    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
    //    SetFontSize(32);
    //    DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
    //    SetFontSize(16);
    //}
}

// ���̃X�e�[�W���擾
StageBase* Stage1::GetNextStage(Player* player)
{
	return new Stage2(player);
}

void Stage1::DrawScrollBackground() const
{
    // === �J�����ӂ���I�t�Z�b�g ===
    static Vector2D camera_offset(0, 0);
    static Vector2D camera_target_offset_prev(0, 0);

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
    Vector2D camera_target_offset = camera_target_offset_prev;

    if (player != nullptr)
    {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }

    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    // ���C���[�ʃI�t�Z�b�g
    Vector2D layer1_offset = camera_offset * 0.3f;  // ���O���b�h
    Vector2D layer2_offset = camera_offset * 1.5f;  // ��O�O���b�h
    Vector2D layer_wall_offset = camera_offset * 0.8f; // �ǃO���b�h

    // === �w�i�O���f�[�V�����i�l�I���O���[����j ===
    for (int y = 0; y < D_WIN_MAX_Y; ++y)
    {
        float t = static_cast<float>(y) / D_WIN_MAX_Y;
        int r = static_cast<int>(5 + (10 - 5) * t);
        int g = static_cast<int>(20 + (60 - 20) * t);
        int b = static_cast<int>(5 + (10 - 5) * t);
        DrawLine(0, y, D_WIN_MAX_X, y, GetColor(r, g, b));
    }

    // === ���q�̐����E�`��i�l�I���O���[���j ===
    while (star_particles.size() < 120)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D((GetRand(200) - 100) * 0.05f, 20.0f + GetRand(30));
        p.alpha = 100.0f + GetRand(155);
        p.length = 6.0f + GetRand(12);
        p.life = 2.0f + (GetRand(100) / 40.0f);
        star_particles.push_back(p);
    }

    for (auto& p : star_particles)
    {
        p.pos += p.velocity * 0.016f;
        p.alpha -= 0.6f;

        if (p.alpha > 0)
        {
            int a = static_cast<int>(p.alpha);
            if (a > 255) a = 255;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
            DrawLine(
                static_cast<int>(p.pos.x - camera_offset.x * 0.6f),
                static_cast<int>(p.pos.y - camera_offset.y * 0.6f),
                static_cast<int>(p.pos.x - camera_offset.x * 0.6f),
                static_cast<int>(p.pos.y + p.length - camera_offset.y * 0.6f),
                GetColor(100, 255, 100));
        }
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p) {
                return (p.pos.y > D_WIN_MAX_Y + 50 || p.alpha <= 0);
            }),
        star_particles.end()
    );

    // === ���̃O���b�h(�_�ł���) ===
    const int grid_size_back = 40;
    int pulse_alpha = static_cast<int>(80 + 50 * sinf(stage_timer * 3.0f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x - (int)layer1_offset.x, 0, x - (int)layer1_offset.x, D_WIN_MAX_Y, GetColor(0, 255, 120));
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy - (int)layer1_offset.y, D_WIN_MAX_X, sy - (int)layer1_offset.y, GetColor(0, 255, 120));
    }

    // === �Ǐ�̏c�O���b�h�i�����}350px�A�ɑ����C���j ===
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    const int wall_grid_size = 50;
    const int wall_thickness = 6; // �ɑ����C��
    const int left_wall_x = (D_WIN_MAX_X / 2) - 350;
    const int right_wall_x = (D_WIN_MAX_X / 2) + 350;

    for (int wall_x : { left_wall_x, right_wall_x })
    {
        for (int y = -wall_grid_size; y < D_WIN_MAX_Y + wall_grid_size; y += wall_grid_size)
        {
            int sy = y - static_cast<int>(bg_scroll_offset_layer1 * 0.4f) % wall_grid_size;
            DrawBox(wall_x - wall_thickness - (int)layer_wall_offset.x,
                sy - (int)layer_wall_offset.y,
                wall_x + wall_thickness - (int)layer_wall_offset.x,
                sy + wall_grid_size - (int)layer_wall_offset.y,
                GetColor(0, 255, 120), TRUE);
        }
    }

    // === ��O�̃O���b�h ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1 - (int)layer2_offset.x, 0, x + 1 - (int)layer2_offset.x, D_WIN_MAX_Y, GetColor(0, 255, 120), TRUE);
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1 - (int)layer2_offset.y, D_WIN_MAX_X, sy + 1 - (int)layer2_offset.y, GetColor(0, 255, 120), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}







//// �w�i�X�N���[���̕`��
//void Stage1::DrawScrollBackground() const
//{
//    // === �J�����ӂ���I�t�Z�b�g�i�v���C���[�ʒu�ɉ����āj ===
//    static Vector2D camera_offset(0, 0);               // �`��p�̂ӂ���I�t�Z�b�g
//    static Vector2D camera_target_offset_prev(0, 0);   // �O��̃^�[�Q�b�g�ʒu�i���S��p�j
//
//    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);
//
//    Vector2D camera_target_offset = camera_target_offset_prev; // �f�t�H���g�͑O��l���g��
//
//    // �v���C���[�����݂��Ă���ΒǏ]�i�ʒu���X�V�j
//    if (player != nullptr)
//    {
//        Vector2D player_pos = player->GetLocation();
//        camera_target_offset = (player_pos - screen_center) * 0.05f;
//        camera_target_offset_prev = camera_target_offset;
//    }
//    else
//    {
//        // ���S��� offset_prev ��ς��Ȃ� �� �Œ莋�_
//        camera_target_offset = camera_target_offset_prev;
//
//    }
//
//    // �I�t�Z�b�g���Ȃ߂炩�ɔ��f
//    camera_offset += (camera_target_offset - camera_offset) * 0.1f;
//
//    // ���C���[�ʃI�t�Z�b�g
//    Vector2D layer1_offset = camera_offset * 0.3f; // ���O���b�h
//    Vector2D layer2_offset = camera_offset * 1.5f; // ��O�O���b�h
//    Vector2D layer3_offset = camera_offset * 0.6f; // ����
//
//
//    // ====== �ȉ��͕`�揈�����̂܂� ======
//    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);
//
//    // === ���q���� ===
//    while (star_particles.size() < 100)
//    {
//        StarParticle p;
//        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
//        p.velocity = Vector2D(0, 40.0f + GetRand(60));
//        p.alpha = 150.0f + GetRand(100);
//        p.length = 10.0f + GetRand(10);
//        p.life = 2.0f + (GetRand(100) / 50.0f);
//        star_particles.push_back(p);
//    }
//
//    // === ���q�`�� ===
//    for (auto& p : star_particles)
//    {
//        p.pos.y += p.velocity.y * 0.016f;
//        p.alpha -= 0.5f;
//
//        if (p.length > 0.0f)
//        {
//            int a = static_cast<int>(p.alpha);
//            if (a < 0) a = 0;
//            if (a > 255) a = 255;
//
//            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
//            DrawLine(
//                static_cast<int>(p.pos.x - layer3_offset.x),
//                static_cast<int>(p.pos.y - layer3_offset.y),
//                static_cast<int>(p.pos.x - layer3_offset.x),
//                static_cast<int>(p.pos.y + p.length - layer3_offset.y),
//                GetColor(200, 255, 255));
//        }
//    }
//
//    star_particles.erase(
//        std::remove_if(star_particles.begin(), star_particles.end(), [](const StarParticle& p)
//            {
//                return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0);
//            }),
//        star_particles.end()
//    );
//
//    const int grid_size_back = 40;
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
//    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
//        DrawLine(x - (int)layer1_offset.x, 0, x - (int)layer1_offset.x, D_WIN_MAX_Y, GetColor(0, 100, 255));
//    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
//    {
//        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
//        DrawLine(0, sy - (int)layer1_offset.y, D_WIN_MAX_X, sy - (int)layer1_offset.y, GetColor(0, 100, 255));
//    }
//
//    const int grid_size_front = 80;
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
//    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
//        DrawBox(x - 1 - (int)layer2_offset.x, 0, x + 1 - (int)layer2_offset.x, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
//    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
//    {
//        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
//        DrawBox(0, sy - 1 - (int)layer2_offset.y, D_WIN_MAX_X, sy + 1 - (int)layer2_offset.y, GetColor(180, 0, 255), TRUE);
//    }
//
//    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
//}


void Stage1::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    enemy_spawn_timer += delta_second;

    if (debug_boss_only)
    {
        if (!boss_spawned)
        {
            const float CENTER_X = D_WIN_MAX_X / 2;
            boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
            boss->SetPattern(BossPattern::Entrance);
            boss->SetPlayer(player);
            boss_spawned = true;
        }
        return;
    }
    if (boss != nullptr) return;

    const float CENTER_X = D_WIN_MAX_X / 2;
    const float CENTER_Y = D_WIN_MAX_Y / 2;

    // ==== Wave1 : Enemy1 �g��˓� ====
    if (stage_timer < 10.0f)
    {
        const int enemy_count = 6;
        const float spacing_x = 100.0f;
        const float appear_y = -80.0f;
        const float target_y = 150.0f;

        if (!spawning)
        {
            spawning = true;
            spawn_index = 0;
            spawn_delay_timer = 0.0f;
        }

        if (spawning)
        {
            spawn_delay_timer -= delta_second;
            if (spawn_delay_timer <= 0.0f && spawn_index < enemy_count)
            {
                float offset_x = (spawn_index - (enemy_count - 1) / 2.0f) * spacing_x;
                Vector2D start_pos(CENTER_X + offset_x, appear_y - (spawn_index % 2) * 50);
                Vector2D target_pos(CENTER_X + offset_x, target_y + (spawn_index % 2) * 40);

                auto zako = objm->CreateObject<Enemy1>(start_pos);
                if (zako)
                {
                    float appear_time = 0.8f + GetRand(10) * 0.05f;
                    zako->SetAppearParams(start_pos, target_pos, appear_time);
                    zako->SetPlayer(player);
                }
                spawn_index++;
                spawn_delay_timer = 0.4f;
            }
            if (spawn_index >= enemy_count) spawning = false;
        }
    }
    // ==== Wave2 : Enemy2 �A�� ====
    else if (stage_timer < 25.0f)
    {
        if (!e2_line_enabled) {
            e2_line_enabled = true;
        }

        enum class ClusterSide { Left, Right, Center };
        enum class LayoutPattern { SnakeHigh, StairsLow };

        auto pickCluster = [&](int group_id)->ClusterSide {
            switch (group_id % 6) {
            case 0: return ClusterSide::Center;
            case 1: return ClusterSide::Left;
            case 2: return ClusterSide::Right;
            case 3: return ClusterSide::Center;
            case 4: return ClusterSide::Right;
            default:return ClusterSide::Left;
            }
            };
        auto pickPattern = [&](int group_id)->LayoutPattern {
            switch (group_id % 4) {
            case 2:  return LayoutPattern::StairsLow;
            default: return LayoutPattern::SnakeHigh;
            }
            };

        const float ANCHOR_LEFT = CENTER_X - 100.0f;
        const float ANCHOR_RIGHT = CENTER_X + 100.0f;
        const float ANCHOR_CENTER = CENTER_X;

        if (e2_line_enabled && stage_timer >= e2_group_next) {
            const int   group_size = 9;
            const float line_interval = 0.14f;
            const float appear_time = 1.00f;
            const float spacing_x = 50.0f;
            const float appear_y = D_WIN_MAX_Y + 100.0f;

            ClusterSide   side = pickCluster(e2_group_id);
            LayoutPattern pattern = pickPattern(e2_group_id);

            const float base_target_y =
                (pattern == LayoutPattern::SnakeHigh) ? (D_WIN_MAX_Y / 2.0f - 120.0f)
                : (D_WIN_MAX_Y / 2.0f + 40.0f);

            float anchor_x =
                (side == ClusterSide::Left) ? ANCHOR_LEFT :
                (side == ClusterSide::Right) ? ANCHOR_RIGHT : ANCHOR_CENTER;

            float base_x = anchor_x - (spacing_x * (group_size - 1) * 0.5f);

            auto patternOffset = [&](int i)->Vector2D {
                float c = i - (group_size - 1) * 0.5f;
                switch (pattern) {
                case LayoutPattern::SnakeHigh: {
                    float ox = 18.0f * sinf(c * 0.9f);
                    float oy = -10.0f * i + 48.0f * sinf(c * 0.9f + DX_PI / 3);
                    return Vector2D(ox, oy);
                }
                case LayoutPattern::StairsLow: {
                    float ox = 0.0f;
                    float oy = +28.0f * i;
                    return Vector2D(ox, oy);
                }
                }
                return Vector2D(0, 0);
                };

            for (int i = 0; i < group_size; ++i) {
                Vector2D ofs = patternOffset(i);
                float x = base_x + i * spacing_x + ofs.x;

                Vector2D s(x, appear_y + i * 20.0f);
                Vector2D t(x, base_target_y + ofs.y);

                auto e2 = objm->CreateObject<Enemy2>(s);
                if (e2) {
                    e2->SetMode(Enemy2Mode::LineRise);
                    e2->SetLineParams(i, group_size, line_interval);
                    e2->SetAppearParams(s, t, appear_time, true);
                    e2->SetPlayer(player);
                }
            }

            float group_finish = stage_timer + (group_size - 1) * line_interval + appear_time;
            e2_single_next = group_finish + 0.15f;
            e2_group_next = group_finish + 0.45f;

            e2_group_id++;
        }

        if (e2_line_enabled && stage_timer >= e2_single_next) {
            ClusterSide   side_for_single = pickCluster(e2_group_id > 0 ? (e2_group_id - 1) : 0);
            LayoutPattern pattern_for_single = pickPattern(e2_group_id > 0 ? (e2_group_id - 1) : 0);

            float anchor_x =
                (side_for_single == ClusterSide::Left) ? ANCHOR_LEFT :
                (side_for_single == ClusterSide::Right) ? ANCHOR_RIGHT : ANCHOR_CENTER;

            float x = anchor_x;
            float single_y = (pattern_for_single == LayoutPattern::SnakeHigh)
                ? (D_WIN_MAX_Y / 2.0f - 125.0f)
                : (D_WIN_MAX_Y / 2.0f + 45.0f);

            Vector2D s(x, D_WIN_MAX_Y + 100.0f);
            Vector2D t(x, single_y);

            auto e2 = objm->CreateObject<Enemy2>(s);
            if (e2) {
                e2->SetMode(Enemy2Mode::Zako3Like);
                e2->SetAppearParams(s, t, 1.00f, true);
                e2->SetPlayer(player);
            }
            e2_single_next = stage_timer + e2_single_interval;
        }

        if (!zako_spawned)
        {
            zako_spawned = true;

            const int   enemy_count = 9;
            const float spacing_x = 50.0f;
            const float appear_y = D_WIN_MAX_Y + 100.0f;
            const float target_y0 = (D_WIN_MAX_Y / 2.0f) - 120.0f;

            float base_x = ANCHOR_CENTER - (spacing_x * (enemy_count - 1) * 0.5f);

            auto snakeOffset = [&](int i)->Vector2D {
                float c = i - (enemy_count - 1) * 0.5f;
                float ox = 18.0f * sinf(c * 0.9f);
                float oy = -10.0f * i + 48.0f * sinf(c * 0.9f + DX_PI / 3);
                return Vector2D(ox, oy);
                };

            for (int i = 0; i < enemy_count; ++i) {
                Vector2D ofs = snakeOffset(i);
                float x = base_x + i * spacing_x + ofs.x;

                Vector2D start_pos(x, appear_y + i * 20.0f);
                Vector2D target_pos(x, target_y0 + ofs.y);

                auto e2 = objm->CreateObject<Enemy2>(start_pos);
                if (e2) {
                    e2->SetAppearParams(start_pos, target_pos, 1.00f, true);
                    e2->SetPlayer(player);
                }
            }
        }
    }
    // ==== Enemy4�i���{�X���U�R�j====
else if (stage_timer < 37.0f)
{
    if (!enemy4_spawned && stage_timer >= 25.0f)
    {
        auto e4 = objm->CreateObject<Enemy4>(Vector2D(CENTER_X, -220.0f));
        if (e4)
        {
            e4->SetPlayer(player);
            enemy4 = e4;                  // �� �Q�Ƃ�ێ�
        }
        enemy4_spawned = true;
    }
}
    // ==== Wave3 : Enemy3 ���� ====
    else if (stage_timer < 44.0f)
    {
        // �� Wave3 �˓����_�Œ��{�X��ޏꂳ����
        if (enemy4) {
            enemy4->ForceRetreat();           // �� 1��Ăׂ�OK�i�p�������ɂ���j
            enemy4 = nullptr;                 // �Q�Ƃ̓N���A���Ă���
        }

        if (!spawning_left && enemy_spawn_timer >= 1.5f)
        {
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 0.0f;
            spawn_delay_timer_right = 0.8f;
            spawning_left = spawning_right = true;
            enemy_spawn_timer = 0.0f;
        }

        if (spawning_left)
        {
            spawn_delay_timer_left -= delta_second;
            if (spawn_delay_timer_left <= 0.0f)
            {
                float x = -80.0f;
                float y = CENTER_Y - 80 + spawn_index_left * 80;

                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
                e3->SetMode(ZakoMode::Zako2);
                e3->SetAppearParams(Vector2D(x, y), Vector2D(D_WIN_MAX_X + 80.0f, y + 50), 2.0f, true);
                e3->SetPlayer(player);

                spawn_index_left++;
                if (spawn_index_left >= 3) spawning_left = false;
                else spawn_delay_timer_left = 1.0f;
            }
        }

        if (spawning_right)
        {
            spawn_delay_timer_right -= delta_second;
            if (spawn_delay_timer_right <= 0.0f)
            {
                float x = CENTER_X - 100 + spawn_index_right * 100;
                float y = D_WIN_MAX_Y + 100.0f;

                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
                e3->SetMode(ZakoMode::Zako7);
                e3->SetAppearParams(Vector2D(x, y), Vector2D(x, CENTER_Y - 100), 2.2f, true);
                e3->SetPlayer(player);

                spawn_index_right++;
                if (spawn_index_right >= 3) spawning_right = false;
                else spawn_delay_timer_right = 1.2f;
            }
        }
    }
    // ==== �A�C�e���x�e ====
    else if (stage_timer < 45.0f)
    {
        if (!spawned_enemy1)
        {
            spawned_enemy1 = true;
           // objm->CreateObject<PowerUp>(Vector2D(CENTER_X - 80, CENTER_Y))->SetPlayer(player);
            objm->CreateObject<Shield>(Vector2D(CENTER_X + 80, CENTER_Y))->SetPlayer(player);
        }
    }
    // ==== Boss ====
    else if (stage_timer > 50.0f && !boss_spawned)
    {
        boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
        boss->SetPattern(BossPattern::Entrance);
        boss->SetPlayer(player);
        boss_spawned = true;
    }
}


//�G�o������
//void Stage1::EnemyAppearance(float delta_second)
//{
//    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
//    enemy_spawn_timer += delta_second;
//
//    if (debug_boss_only)
//    {
//        if (!boss_spawned)
//        {
//            const float CENTER_X = D_WIN_MAX_X / 2;
//            boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
//            boss->SetPattern(BossPattern::Entrance);
//            boss->SetPlayer(player);
//            boss_spawned = true;
//        }
//        return;
//    }
//    if (boss != nullptr) return;
//
//    const float CENTER_X = D_WIN_MAX_X / 2;
//    const float CENTER_Y = D_WIN_MAX_Y / 2;
//
//    // ==== Wave1 : Enemy1 �g��˓� ====
//    if (stage_timer < 10.0f)
//    {
//        const int enemy_count = 6;
//        const float spacing_x = 100.0f;
//        const float appear_y = -80.0f;
//        const float target_y = 150.0f;
//
//        if (!spawning)
//        {
//            spawning = true;
//            spawn_index = 0;
//            spawn_delay_timer = 0.0f;
//        }
//
//        if (spawning)
//        {
//            spawn_delay_timer -= delta_second;
//            if (spawn_delay_timer <= 0.0f && spawn_index < enemy_count)
//            {
//                float offset_x = (spawn_index - (enemy_count - 1) / 2.0f) * spacing_x;
//                Vector2D start_pos(CENTER_X + offset_x, appear_y - (spawn_index % 2) * 50);
//                Vector2D target_pos(CENTER_X + offset_x, target_y + (spawn_index % 2) * 40);
//
//                auto zako = objm->CreateObject<Enemy1>(start_pos);
//                if (zako)
//                {
//                    float appear_time = 0.8f + GetRand(10) * 0.05f;
//                    zako->SetAppearParams(start_pos, target_pos, appear_time);
//                    zako->SetPlayer(player);
//                }
//                spawn_index++;
//                spawn_delay_timer = 0.4f;
//            }
//            if (spawn_index >= enemy_count) spawning = false;
//        }
//    }
//
//
//    // ==== Wave2 : Enemy2 �X�l�[�N�ヌ�[���^�K�i�����[���i�������E���ԕۏ؁E���Ȃ��j ====
//    else if (stage_timer < 25.0f)
//    {
//        if (!e2_line_enabled) {
//            e2_line_enabled = true;
//        }
//
//        enum class ClusterSide { Left, Right, Center };
//        enum class LayoutPattern { SnakeHigh, StairsLow };
//
//        // �o�����X�ǂ��񂹏��FC �� L �� R �� C �� R �� L �c
//        auto pickCluster = [&](int group_id)->ClusterSide {
//            switch (group_id % 6) {
//            case 0: return ClusterSide::Center;
//            case 1: return ClusterSide::Left;
//            case 2: return ClusterSide::Right;
//            case 3: return ClusterSide::Center;
//            case 4: return ClusterSide::Right;
//            default:return ClusterSide::Left;
//            }
//            };
//        // �p�^�[���́u�ցE�ցE�K�i�E�ցc�v�Łg�ցh�������
//        auto pickPattern = [&](int group_id)->LayoutPattern {
//            switch (group_id % 4) {
//            case 2:  return LayoutPattern::StairsLow; // 3��1�񂾂��K�i
//            default: return LayoutPattern::SnakeHigh;
//            }
//            };
//
//        const float CENTER_X = D_WIN_MAX_X / 2;
//        const float ANCHOR_LEFT = CENTER_X - 100.0f; // ��ʓ��Ɏ��߂�����A���J�[
//        const float ANCHOR_RIGHT = CENTER_X + 100.0f;
//        const float ANCHOR_CENTER = CENTER_X;
//
//        if (e2_line_enabled && stage_timer >= e2_group_next) {
//            // --- �e���|�ƌ����ځi�g�������ł����ԁh��ۏ؁j ---
//            const int   group_size = 9;       // ��������
//            const float line_interval = 0.14f;   // ������ɑ����ii�~interval �ŏ��ԕۏ؁j:contentReference[oaicite:4]{index=4}
//            const float appear_time = 1.00f;   // �S���Œ莞�Ԃœ��B:contentReference[oaicite:5]{index=5}
//            const float spacing_x = 50.0f;   // ���߃N���X�^�[�i�͂ݏo���h�~�j
//            const float appear_y = D_WIN_MAX_Y + 100.0f;
//
//            ClusterSide   side = pickCluster(e2_group_id);
//            LayoutPattern pattern = pickPattern(e2_group_id);
//
//            // ���[���ʂ̊Y�F��=�ヌ�[���i��������j�A�K�i=�����[���i������艺�j
//            const float base_target_y =
//                (pattern == LayoutPattern::SnakeHigh) ? (D_WIN_MAX_Y / 2.0f - 120.0f)
//                : (D_WIN_MAX_Y / 2.0f + 40.0f);
//
//            float anchor_x =
//                (side == ClusterSide::Left) ? ANCHOR_LEFT :
//                (side == ClusterSide::Right) ? ANCHOR_RIGHT : ANCHOR_CENTER;
//
//            // �N���X�^�������A���J�[�Ɉ�v
//            float base_x = anchor_x - (spacing_x * (group_size - 1) * 0.5f);
//
//            // �p�^�[�����Ƃ̐���i�����_���Ȃ��j
//            auto patternOffset = [&](int i)->Vector2D {
//                float c = i - (group_size - 1) * 0.5f; // ���S���i-4..+4�j
//                switch (pattern) {
//                case LayoutPattern::SnakeHigh: {
//                    // �ցi�ヌ�[���j�F�c�U�������߂āg�����h������
//                    float ox = 18.0f * sinf(c * 0.9f);
//                    float oy = -10.0f * i + 48.0f * sinf(c * 0.9f + DX_PI / 3);
//                    return Vector2D(ox, oy);
//                }
//                case LayoutPattern::StairsLow: {
//                    // �K�i�i�����[���j�F�����E�Ɂg�������āh�����i�ւ̎ז������Ȃ��j
//                    float ox = 0.0f;
//                    float oy = +28.0f * i; // �������֒i���i�S�����������Ɏ��܂�j
//                    return Vector2D(ox, oy);
//                }
//                }
//                return Vector2D(0, 0);
//                };
//
//            for (int i = 0; i < group_size; ++i) {
//                Vector2D ofs = patternOffset(i);
//                float x = base_x + i * spacing_x + ofs.x;
//
//                Vector2D s(x, appear_y + i * 20.0f);   // �A�Ȃ��ďオ�錩���ځi�����Ȃ̂ōT���߁j
//                Vector2D t(x, base_target_y + ofs.y);  // ��/�����[���ɉ����Đ���
//
//                auto e2 = objm->CreateObject<Enemy2>(s);
//                if (e2) {
//                    e2->SetMode(Enemy2Mode::LineRise);                    // �񃂁[�h:contentReference[oaicite:6]{index=6}
//                    e2->SetLineParams(i, group_size, line_interval);      // i�~�Ԋu�ŏ��ԑ҂�:contentReference[oaicite:7]{index=7}
//                    e2->SetAppearParams(s, t, appear_time, true);         // �Œ莞�Ԃœ��B:contentReference[oaicite:8]{index=8}
//                    e2->SetPlayer(player);
//                }
//            }
//
//            // �O���[�v������ɂ�����������i���荞�݋֎~�j
//            float group_finish = stage_timer + (group_size - 1) * line_interval + appear_time;
//            e2_single_next = group_finish + 0.15f;  // �P���̓O���[�v��̂�:contentReference[oaicite:9]{index=9}
//            e2_group_next = group_finish + 0.45f;  // ���O���[�v���d�˂Ȃ�:contentReference[oaicite:10]{index=10}
//
//            e2_group_id++;
//        }
//
//        // �P���F���O�N���X�^�̐c�ɒu���i���[���͈ێ��A���Ȃ��j
//        if (e2_line_enabled && stage_timer >= e2_single_next) {
//            ClusterSide   side_for_single = pickCluster(e2_group_id > 0 ? (e2_group_id - 1) : 0);
//            LayoutPattern pattern_for_single = pickPattern(e2_group_id > 0 ? (e2_group_id - 1) : 0);
//
//            float anchor_x =
//                (side_for_single == ClusterSide::Left) ? ANCHOR_LEFT :
//                (side_for_single == ClusterSide::Right) ? ANCHOR_RIGHT : ANCHOR_CENTER;
//
//            float x = anchor_x;
//            float single_y = (pattern_for_single == LayoutPattern::SnakeHigh)
//                ? (D_WIN_MAX_Y / 2.0f - 125.0f)   // �ւȂ�ヌ�[����
//                : (D_WIN_MAX_Y / 2.0f + 45.0f);  // �K�i�Ȃ牺���[����
//
//            Vector2D s(x, D_WIN_MAX_Y + 100.0f);
//            Vector2D t(x, single_y);
//
//            auto e2 = objm->CreateObject<Enemy2>(s);
//            if (e2) {
//                e2->SetMode(Enemy2Mode::Zako3Like);               // ������ʂœ��B:contentReference[oaicite:11]{index=11}
//                e2->SetAppearParams(s, t, 1.00f, true);           // �Œ莞��:contentReference[oaicite:12]{index=12}
//                e2->SetPlayer(player);
//            }
//            e2_single_next = stage_timer + e2_single_interval;    // �����X�P�W���[��:contentReference[oaicite:13]{index=13}
//        }
//
//        // ����͒����E�ւ̂݁i���[���ň�ەt���j
//        if (!zako_spawned)
//        {
//            zako_spawned = true;
//
//            const int   enemy_count = 9;
//            const float spacing_x = 50.0f;
//            const float appear_y = D_WIN_MAX_Y + 100.0f;
//            const float target_y0 = (D_WIN_MAX_Y / 2.0f) - 120.0f; // �ヌ�[��
//
//            float base_x = ANCHOR_CENTER - (spacing_x * (enemy_count - 1) * 0.5f);
//
//            auto snakeOffset = [&](int i)->Vector2D {
//                float c = i - (enemy_count - 1) * 0.5f;
//                float ox = 18.0f * sinf(c * 0.9f);
//                float oy = -10.0f * i + 48.0f * sinf(c * 0.9f + DX_PI / 3);
//                return Vector2D(ox, oy);
//                };
//
//            for (int i = 0; i < enemy_count; ++i) {
//                Vector2D ofs = snakeOffset(i);
//                float x = base_x + i * spacing_x + ofs.x;
//
//                Vector2D start_pos(x, appear_y + i * 20.0f);
//                Vector2D target_pos(x, target_y0 + ofs.y);
//
//                auto e2 = objm->CreateObject<Enemy2>(start_pos);
//                if (e2) {
//                    e2->SetAppearParams(start_pos, target_pos, 1.00f, true); // �Œ莞��:contentReference[oaicite:14]{index=14}
//                    e2->SetPlayer(player);
//                }
//            }
//        }
//    }
//
//    // ==== Enemy4�i���{�X���U�R�j====
//   // 25�b�`35�b�̊Ԃ�1�񂾂��o��
//else if (stage_timer < 35.0f)
//{
//    if (!enemy4_spawned && stage_timer >= 25.0f)
//    {
//        Vector2D start_pos(CENTER_X, D_WIN_MAX_Y + 120.0f);
//        Vector2D end_pos(CENTER_X, CENTER_Y - 80.0f);
//
//        auto e4 = objm->CreateObject<Enemy4>(start_pos); // �� �ʒu��������
//        if (e4)
//        {
//            e4->SetAppearParams(start_pos, end_pos, 1.10f);
//            e4->SetPlayer(player);
//        }
//        enemy4_spawned = true; // �ďo���h�~
//    }
//}
//
//
//    // ==== Wave3 : Enemy3 Zako2 + Zako7 ���� ====
//    else if (stage_timer < 40.0f)
//    {
//        if (!spawning_left && enemy_spawn_timer >= 1.5f)
//        {
//            spawn_index_left = 0;
//            spawn_index_right = 0;
//            spawn_delay_timer_left = 0.0f;
//            spawn_delay_timer_right = 0.8f;
//            spawning_left = spawning_right = true;
//            enemy_spawn_timer = 0.0f;
//        }
//
//        if (spawning_left)
//        {
//            spawn_delay_timer_left -= delta_second;
//            if (spawn_delay_timer_left <= 0.0f)
//            {
//                float x = -80.0f;
//                float y = CENTER_Y - 80 + spawn_index_left * 80;
//
//                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
//                e3->SetMode(ZakoMode::Zako2);
//                e3->SetAppearParams(Vector2D(x, y), Vector2D(D_WIN_MAX_X + 80.0f, y + 50), 2.0f, true);
//                e3->SetPlayer(player);
//
//                spawn_index_left++;
//                if (spawn_index_left >= 3) spawning_left = false;
//                else spawn_delay_timer_left = 1.0f;
//            }
//        }
//
//        if (spawning_right)
//        {
//            spawn_delay_timer_right -= delta_second;
//            if (spawn_delay_timer_right <= 0.0f)
//            {
//                float x = CENTER_X - 100 + spawn_index_right * 100;
//                float y = D_WIN_MAX_Y + 100.0f;
//
//                auto e3 = objm->CreateObject<Enemy3>(Vector2D(x, y));
//                e3->SetMode(ZakoMode::Zako7);
//                e3->SetAppearParams(Vector2D(x, y), Vector2D(x, CENTER_Y - 100), 2.2f, true);
//                e3->SetPlayer(player);
//
//                spawn_index_right++;
//                if (spawn_index_right >= 3) spawning_right = false;
//                else spawn_delay_timer_right = 1.2f;
//            }
//        }
//    }
//
//    // ==== �A�C�e���x�e ====
//    else if (stage_timer < 45.0f)
//    {
//        if (!spawned_enemy1)
//        {
//            spawned_enemy1 = true;
//            objm->CreateObject<PowerUp>(Vector2D(CENTER_X - 80, CENTER_Y))->SetPlayer(player);
//            objm->CreateObject<Shield>(Vector2D(CENTER_X + 80, CENTER_Y))->SetPlayer(player);
//        }
//    }
//
//    // ==== Boss ====
//    else if (stage_timer > 45.0f && !boss_spawned)
//    {
//        boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
//        boss->SetPattern(BossPattern::Entrance);
//        boss->SetPlayer(player);
//        boss_spawned = true;
//    }
//}


//void Stage1::EnemyAppearance(float delta_second)
//{
//    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
//
//    // ���łɃ{�X�������ς݂Ȃ牽�����Ȃ�
//    if (boss_spawned) return;
//
//    // �v���C���[�̈ʒu�𒆉����Ƀ{�X���ɏo��������
//    boss = objm->CreateObject<Boss1>(Vector2D(D_WIN_MAX_X / 2, -200));
//    boss->SetPattern(BossPattern::Entrance);
//    boss->SetPlayer(player);
//
//    boss_spawned = true;  // ��x�����o��������
//}





// �N���A����
// �N���A����
void Stage1::UpdateGameStatus(float delta_second)
{
    // �J�ڎ��̃m�C�Y���o�iStage2�ɍ��킹�����ʁj
    if (entry_effect_playing)
    {
        entry_effect_timer += delta_second;
        if (entry_effect_timer >= 1.0f)
        {
            entry_effect_playing = false;
        }
    }

    // �{�X���|�ꂽ��N���A
    if (boss != nullptr && boss->GetIsAlive() == false && is_over == false)
    {
        boss->SetDestroy();
        is_clear = true;
        // �m�C�Y���o���J�n
        entry_effect_playing = true;
        entry_effect_timer = 0.0f;
    }

    // �v���C���[���|�ꂽ��Q�[���I�[�o�[
    if (player != nullptr && player->GetGameOver() && is_clear == false)
    {
        is_over = true;
        is_finished = true;
    }

    // �X�e�[�W�I�����̓����i�N���A��̑ҋ@�j
    if (is_clear == true)
    {
        scene_timer += delta_second;

        if (scene_timer >= 2.0f)
        {
            is_finished = true;  // GetNextStage()��Stage2�ɑJ��
        }
    }
}


void Stage1::StageLabel() const
{
    if (warning_label_state != WarningLabelState::None && warning_label_band_height > 1.0f)
    {
        int y_top = static_cast<int>(360 - warning_label_band_height);
        int y_bottom = static_cast<int>(360 + warning_label_band_height);

        // �p���X���C�e�B���O�i���Ń��C���j
        float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f; // 0.0?1.0
        int pulse_alpha = static_cast<int>(pulse * 180);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
        DrawLine(0, y_top, 1280, y_top, GetColor(120, 220, 255));
        DrawLine(0, y_bottom, 1280, y_bottom, GetColor(120, 220, 255));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // �O���b�`���̃����_���X�L�������C���i��������o�j
        for (int i = 0; i < 8; ++i)
        {
            int glitch_y = y_top + rand() % (2 * static_cast<int>(warning_label_band_height));
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (1280 - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
        if (
            warning_label_state == WarningLabelState::Displaying ||
            warning_label_state == WarningLabelState::SlideOut
            )
        {
            int center_x = D_WIN_MAX_X / 2;

            // �X���C�h�C��
            float slide_in_t = warning_label_timer / 0.5f;
            if (slide_in_t > 1.0f) slide_in_t = 1.0f;
            float slide_in = 1.0f - powf(1.0f - slide_in_t, 3.0f);

            // �X���C�h�A�E�g�i�E�����j
            float slide_out_t = slide_out_timer / 0.5f;
            if (slide_out_t > 1.0f) slide_out_t = 1.0f;
            float slide_out = 1.0f + slide_out_t;  // ? ���֗����␳

            float t = (warning_label_state == WarningLabelState::Displaying) ? slide_in : slide_out;

            int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
            int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

            DrawStringToHandle(stage_name_x, 320,
                "Neural Grid", GetColor(255, 255, 255), font_warning);

            DrawStringToHandle(sub_text_x, 370,
                "Eliminate all hostile units.", GetColor(120, 255, 255), font_orbitron);
        }
    }
}

// �v���C���[�̓o�ꏈ��
void Stage1::AppearancePlayer(float delta_second)
{
    // �v���C���[�̓o��̓���
    if (is_player_entering)
    {
        player->SetShotStop(true);
        const float duration = 2.5f; // �������2.5�b�����č~��
        player_entry_timer += delta_second;

        float t = player_entry_timer / duration;
        if (t >= 1.0f)
        {
            t = 1.0f;
            is_player_entering = false;
            player->SetMobility(true);
            player->ForceNeutralAnim(false); // �� �A�j���[�V��������
            player->SetShotStop(false);
        }

        // �ɂ₩�ɉ��~����C�[�W���O�ieaseOutQuad�j
        float eased = 1.0f - (1.0f - t) * (1.0f - t);
        Vector2D new_pos = entry_start_pos + (entry_end_pos - entry_start_pos) * eased;
        player->SetLocation(new_pos);
    }
}