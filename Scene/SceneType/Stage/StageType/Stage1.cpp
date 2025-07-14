#include "Stage1.h"
#include "Stage2.h"
#include "../../../../Object/Character/Enemy/Enemy1.h"
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

    // �Q�[���I�[�o�[���̉��o
    if (is_over)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
        SetFontSize(16);
    }
}

// ���̃X�e�[�W���擾
StageBase* Stage1::GetNextStage(Player* player)
{
	return new Stage2(player);
}

// �w�i�X�N���[���̕`��
void Stage1::DrawScrollBackground() const
{
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(20, 20, 40), TRUE);

    // === ���q���� ===
    while (star_particles.size() < 100)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 40.0f + GetRand(60)); // ��⑬��
        p.alpha = 150.0f + GetRand(100); // ������
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

            // ��薾�邢�F
            DrawLine(static_cast<int>(p.pos.x),
                static_cast<int>(p.pos.y),
                static_cast<int>(p.pos.x),
                static_cast<int>(p.pos.y + p.length),
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
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(0, 100, 255));
    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(0, 100, 255));
    }

    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(180, 0, 255), TRUE);
    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front)
    {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(180, 0, 255), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// �G�̏o����stage_timer�ŊǗ�
void Stage1::EnemyAppearance(float delta_second)
{
    enemy_spawn_timer += delta_second;

    float spawn_interval = 2.0f - (stage_timer / 5.0f);
    if (spawn_interval < 0.5f) spawn_interval = 0.5f;

    if (enemy_spawn_timer >= spawn_interval)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        // �y0?24�b�z�F�c���璼���I�ɏo��
        if (stage_timer < 24.0f)
        {
            const int lane_x[] = { 250, 350, 450, 550, 650, 750, 850 };
            float x = static_cast<float>(lane_x[GetRand(7)]);
            Enemy1* enemy = objm->CreateObject<Enemy1>(Vector2D(x, -40));
            enemy->SetPattern(Enemy1Pattern::MoveStraight);
            enemy->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        // �y25?48�b�z�F���E�����荞�݈ړ�
        else if (stage_timer < 48.0f)
        {
            const int lane_y[] = { 40, 120, 200 };
            float y = static_cast<float>(lane_y[GetRand(3)]);
            bool from_left = fmod(stage_timer, 20.0f) < 10.0f;
            Vector2D pos = from_left ? Vector2D(270.0f, y) : Vector2D(1000.0f, y);
            Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

            Enemy1* zako = objm->CreateObject<Enemy1>(pos);
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        // �y49?59�b�z�F�ˌ��n�p�^�[���i�~�܂��Č��� or �ːi�j
        else if (stage_timer < 60.0f)
        {
            const int lane_x[] = { 350, 450, 550, 650, 750, 850 };
            float x = static_cast<float>(lane_x[GetRand(6)]);
            Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, -40));
            Enemy1Pattern pattern = (GetRand(2) == 0)
                ? Enemy1Pattern::MoveAndStopShoot
                : Enemy1Pattern::DiveOnce;
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        else if (stage_timer < 65.0f) // �y60?65�b�z�FZIgzag + MoveStraight ����
        {
            for (int i = 0; i < 5; ++i)
            {
                float x = 300.0f + GetRand(400);
                Enemy1Pattern pattern = (i % 2 == 0) ? Enemy1Pattern::ZIgzag : Enemy1Pattern::MoveStraight;

                Enemy1* zako = objm->CreateObject<Enemy1>(Vector2D(x, -60.0f - i * 30));
                zako->SetPattern(pattern);
                zako->SetPlayer(player);
                //enemy_list.push_back(zako);
            }
        }

        else if (stage_timer < 70.0f) // �y65?70�b�z�F���E���珇�Ԃɓo��
        {
            const int lane_y[] = { 40, 120, 200 };
            float y = static_cast<float>(lane_y[GetRand(3)]);
            bool from_left = fmod(stage_timer, 20.0f) < 10.0f;
            Vector2D pos = from_left ? Vector2D(270.0f, y) : Vector2D(1000.0f, y);
            Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

            Enemy1* zako = objm->CreateObject<Enemy1>(pos);
            zako->SetPattern(pattern);
            zako->SetPlayer(player);
            //enemy_list.push_back(zako);
        }

        else if (stage_timer < 74.0f) // �y70?74�b�z�F�����������߂�悤�Ɉ�C�ɓːi
        {
            for (int i = 0; i < 3; ++i)
            {
                bool from_left = (GetRand(2) == 0);
                float y = 100.0f + i * 80.0f;

                Vector2D pos = from_left
                    ? Vector2D(-60.0f, y)
                    : Vector2D(D_WIN_MAX_X + 60.0f, y);

                Enemy1Pattern pattern = from_left ? Enemy1Pattern::RightMove : Enemy1Pattern::LeftMove;

                Enemy1* zako = objm->CreateObject<Enemy1>(pos);
                zako->SetPattern(pattern);
                zako->SetPlayer(player);
                //enemy_list.push_back(zako);
            }
        }

        //else if (stage_timer < 77.0f) // �y74?77�b�z�F�O���b�`���o �� �S�U�R���ޔ�
        //{
        //    for (auto& enemy : enemy_list)
        //    {
        //        if (!enemy || enemy->is_destroy) continue;

        //        Zako* zako = dynamic_cast<Zako*>(enemy);
        //        if (zako == nullptr) continue;

        //        ZakoPattern current = zako->GetPattern();

        //        if (current == ZakoPattern::MoveAndStopShoot ||
        //            current == ZakoPattern::DiveOnce ||
        //            current == ZakoPattern::RotateAndShoot ||
        //            current == ZakoPattern::PauseThenRush ||
        //            current == ZakoPattern::LeftMove ||
        //            current == ZakoPattern::RightMove ||
        //            current == ZakoPattern::Formation)
        //        {
        //            zako->SetPattern(ZakoPattern::RetreatUp);
        //        }
        //    }
        //}

        // �y77�b�ȍ~�z�F�{�X��˓� + �A�C�e�����o
        else
        {
            if (!boss_spawned)
            {
                objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
                objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2 + 60, 120))->SetPlayer(player);

                boss = objm->CreateObject<Boss1>(Vector2D(670, -200));
                boss->SetPattern(BossPattern::Entrance);
                boss->SetPlayer(player);
                //enemy_list.push_back(boss1);

                boss_spawned = true;
                //is_warning = false;
            }
        }

        enemy_spawn_timer = 0.0f;
    }
}

// �N���A����
void Stage1::UpdateGameStatus(float delta_second)
{
    // �{�X���|�ꂽ��N���A
    if (boss != nullptr && boss->GetIsAlive() == false && is_over == false)
    {
        boss->SetDestroy();
        is_clear = true;
    }

    // �v���C���[���|�ꂽ��Q�[���I�[�o�[
    if (player != nullptr && player->GetIsAlive() == false && is_clear == false)
    {
        // �I�u�W�F�N�g�Ǘ��N���X�̃C���X�^���X���擾
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
        objm->Finalize();
        is_over = true;
    }

    // �X�e�[�W�I�����̓���
    if (is_clear == true)
    {
        // �����ҋ@������I��
        scene_timer += delta_second;

        if (scene_timer >= 2.0f)
        {
            is_finished = true;
        }
    }
    else if (is_over == true)
    {
        // �����ҋ@������I��
        scene_timer += delta_second;
        trans_timer += delta_second;

        if (trans_timer >= 0.02f)
            trans_timer = 0.0f;
        if (transparent < 255)
            transparent++;

        if (scene_timer >= 7.0f)
        {
            is_finished = true;
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