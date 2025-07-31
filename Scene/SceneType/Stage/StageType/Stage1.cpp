#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
#include "../../../../Object/Character/Enemy/Enemy2.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
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

// �w�i�X�N���[���̕`��
void Stage1::DrawScrollBackground() const
{
    // === �J�����ӂ���I�t�Z�b�g�i�v���C���[�ʒu�ɉ����āj ===
    static Vector2D camera_offset(0, 0);               // �`��p�̂ӂ���I�t�Z�b�g
    static Vector2D camera_target_offset_prev(0, 0);   // �O��̃^�[�Q�b�g�ʒu�i���S��p�j

    Vector2D screen_center(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2);

    Vector2D camera_target_offset = camera_target_offset_prev; // �f�t�H���g�͑O��l���g��

    // �v���C���[�����݂��Ă���ΒǏ]�i�ʒu���X�V�j
    if (player != nullptr)
    {
        Vector2D player_pos = player->GetLocation();
        camera_target_offset = (player_pos - screen_center) * 0.05f;
        camera_target_offset_prev = camera_target_offset;
    }
    else
    {
        // ���S��� offset_prev ��ς��Ȃ� �� �Œ莋�_
        camera_target_offset = camera_target_offset_prev;

    }

    // �I�t�Z�b�g���Ȃ߂炩�ɔ��f
    camera_offset += (camera_target_offset - camera_offset) * 0.1f;

    // ���C���[�ʃI�t�Z�b�g
    Vector2D layer1_offset = camera_offset * 0.3f; // ���O���b�h
    Vector2D layer2_offset = camera_offset * 1.5f; // ��O�O���b�h
    Vector2D layer3_offset = camera_offset * 0.6f; // ����


    // ====== �ȉ��͕`�揈�����̂܂� ======
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);

    // === ���q���� ===
    while (star_particles.size() < 100)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 40.0f + GetRand(60));
        p.alpha = 150.0f + GetRand(100);
        p.length = 10.0f + GetRand(10);
        p.life = 2.0f + (GetRand(100) / 50.0f);
        star_particles.push_back(p);
    }

    // === ���q�`�� ===
    for (auto& p : star_particles)
    {
        p.pos.y += p.velocity.y * 0.016f;
        p.alpha -= 0.5f;

        if (p.length > 0.0f)
        {
            int a = static_cast<int>(p.alpha);
            if (a < 0) a = 0;
            if (a > 255) a = 255;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
            DrawLine(
                static_cast<int>(p.pos.x - layer3_offset.x),
                static_cast<int>(p.pos.y - layer3_offset.y),
                static_cast<int>(p.pos.x - layer3_offset.x),
                static_cast<int>(p.pos.y + p.length - layer3_offset.y),
                GetColor(200, 255, 255));
        }
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(), [](const StarParticle& p)
            {
                return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0);
            }),
        star_particles.end()
    );

    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x - (int)layer1_offset.x, 0, x - (int)layer1_offset.x, D_WIN_MAX_Y, GetColor(0, 100, 255));
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy - (int)layer1_offset.y, D_WIN_MAX_X, sy - (int)layer1_offset.y, GetColor(0, 100, 255));
    }

    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1 - (int)layer2_offset.x, 0, x + 1 - (int)layer2_offset.x, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1 - (int)layer2_offset.y, D_WIN_MAX_X, sy + 1 - (int)layer2_offset.y, GetColor(180, 0, 255), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}


//�G�o������
void Stage1::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    enemy_spawn_timer += delta_second;

    //�f�o�b�O�p�Febug_boss_only��true�@�Ȃ�{�X�̂ݏo������
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
        return; // �G���o���������΂�
    }

    //�ʏ�̃X�e�[�W�P�̓G�o������
    if (boss != nullptr) return;

    const float CENTER_X = D_WIN_MAX_X / 2;
    const float CENTER_Y = D_WIN_MAX_Y / 2;

    // === Wave1:(Enemy1) ===
    if (stage_timer < 7.0f)
    {
        const int num = 4;
        const float spacing_y = 100.0f;  // Y�Ԋu���L��
        const float appear_y = -80.0f;

        if (!spawning_left && enemy_spawn_timer >= 3.0f)
        {
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 0.0f;
            spawn_delay_timer_right = 0.6f;   // �E�͂���ɒx�点��
            spawning_left = spawning_right = true;
            enemy_spawn_timer = 0.0f;
        }

        // �����珇�Ԃ�
        if (spawning_left)
        {
            spawn_delay_timer_left -= delta_second;
            if (spawn_delay_timer_left <= 0.0f)
            {
                float x = CENTER_X - 200.0f;
                float y = appear_y;

                auto zako = objm->CreateObject<Enemy1>(Vector2D(x, y));
                zako->SetAppearParams(Vector2D(x, y),
                    Vector2D(CENTER_X + 100, 120 + spawn_index_left * spacing_y), 1.8f); // �X�s�[�h�x��
                zako->SetPlayer(player);

                spawn_index_left++;
                if (spawn_index_left >= num) spawning_left = false;
                else spawn_delay_timer_left = 0.7f; // �Ԋu���L��
            }
        }

        // �E���珇�Ԃ�
        if (spawning_right)
        {
            spawn_delay_timer_right -= delta_second;
            if (spawn_delay_timer_right <= 0.0f)
            {
                float x = CENTER_X + 200.0f;
                float y = appear_y;

                auto zako = objm->CreateObject<Enemy1>(Vector2D(x, y));
                zako->SetAppearParams(Vector2D(x, y),
                    Vector2D(CENTER_X - 100, 120 + spawn_index_right * spacing_y), 1.8f); // �X�s�[�h�x��
                zako->SetPlayer(player);

                spawn_index_right++;
                if (spawn_index_right >= num) spawning_right = false;
                else spawn_delay_timer_right = 0.7f;
            }
        }
    }

    // === Wave2:(Enemy3 Zako2) ===
    else if (stage_timer < 14.0f)
    {
        const int num = 3;
        const float spacing = 100.0f;  // Y�Ԋu���L��

        if (!spawning_left && enemy_spawn_timer >= 3.0f)
        {
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 0.0f;
            spawn_delay_timer_right = 0.8f; // �c�����͂���ɒx�点��
            spawning_left = spawning_right = true;
            enemy_spawn_timer = 0.0f;
        }

        // �����������Ԃ�
        if (spawning_left)
        {
            spawn_delay_timer_left -= delta_second;
            if (spawn_delay_timer_left <= 0.0f)
            {
                float x = -80.0f;
                float y = CENTER_Y - 100 + spawn_index_left * spacing;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, y));
                zako->SetMode(ZakoMode::Zako2);
                zako->SetAppearParams(Vector2D(x, y), Vector2D(D_WIN_MAX_X + 80.0f, y), 3.0f, true); // �X�s�[�h�x��
                zako->SetPlayer(player);

                spawn_index_left++;
                if (spawn_index_left >= num) spawning_left = false;
                else spawn_delay_timer_left = 0.9f; // �o���Ԋu���L����
            }
        }

        // �c���������Ԃ�
        if (spawning_right)
        {
            spawn_delay_timer_right -= delta_second;
            if (spawn_delay_timer_right <= 0.0f)
            {
                float x = CENTER_X - 140 + spawn_index_right * spacing;
                float y = -60.0f;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, y));
                zako->SetMode(ZakoMode::Zako2);
                zako->SetAppearParams(Vector2D(x, y), Vector2D(x, D_WIN_MAX_Y + 80.0f), 3.0f, true);
                zako->SetPlayer(player);

                spawn_index_right++;
                if (spawn_index_right >= num) spawning_right = false;
                else spawn_delay_timer_right = 0.9f;
            }
        }
    }

    // === Wave3:(Enemy3 Zako7) ===
    else if (stage_timer < 36.0f)
    {
        const int num = 4;                   // �������炷
        const float spacing = 130.0f;        // �Ԋu�L��
        const float base_x = (D_WIN_MAX_X / 2) - (spacing * (num - 1) / 2);

        if (!spawning_left && enemy_spawn_timer >= 3.0f)
        {
            spawn_index_left = 0;
            spawn_index_right = 0;
            spawn_delay_timer_left = 3.8f;
            spawn_delay_timer_right = 1.0f; // ��������x�点��
            spawning_left = spawning_right = true;
            enemy_spawn_timer = 0.0f;
        }

        // �ォ�珇�Ԃ�
        if (spawning_left)
        {
            spawn_delay_timer_left -= delta_second;
            if (spawn_delay_timer_left <= 0.0f)
            {
                float x = base_x + spawn_index_left * spacing;
                float y = -100.0f;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, y), Vector2D(x, CENTER_Y + 120), 2.8f, true);
                zako->SetPlayer(player);

                spawn_index_left++;
                if (spawn_index_left >= num) spawning_left = false;
                else spawn_delay_timer_left = 3.0f; // �o���Ԋu���L����
            }
        }

        // �����珇�Ԃ�
        if (spawning_right)
        {
            spawn_delay_timer_right -= delta_second;
            if (spawn_delay_timer_right <= 0.0f)
            {
                float x = base_x + spawn_index_right * spacing;
                float y = D_WIN_MAX_Y + 100.0f;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, y), Vector2D(x, CENTER_Y - 120), 2.8f, true);
                zako->SetPlayer(player);

                spawn_index_right++;
                if (spawn_index_right >= num) spawning_right = false;
                else spawn_delay_timer_right = 3.8f;
            }
        }
    }

    // === Boss�o�� ===
    else if (stage_timer > 40.0f && !boss_spawned)
    {
        objm->CreateObject<PowerUp>(Vector2D(CENTER_X - 80, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(CENTER_X + 80, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss1>(Vector2D(CENTER_X, -200));
        boss->SetPattern(BossPattern::Entrance);
        boss->SetPlayer(player);
        boss_spawned = true;
    }
}

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