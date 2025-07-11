#include "Stage3.h"
#include "Stage4.h"
#include "../../../../Object/Character/Enemy/Enemy3.h"
#include "../../../../Object/Character/Boss/Boss3.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage3::Stage3(Player* player)
    : StageBase(player)
{
}

Stage3::~Stage3()
{
}

// ����������
void Stage3::Initialize()
{
    stage_id = StageID::Stage3;
}

// �I��������
void Stage3::Finalize()
{
}

// �X�V����
void Stage3::Update(float delta_second)
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

    // �N���A����
    UpdateGameStatus(delta_second);
    
    // �X�N���[���̍X�V����
    UpdateBackgroundScroll(delta_second);
    ScrollEffectUpdate(delta_second);
}

// �`�揈��
void Stage3::Draw()
{
    // �w�i�X�N���[���̕`��
    DrawScrollBackground(); 

    // �C���X�^���X�̎擾
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    EffectManager* em = Singleton<EffectManager>::GetInstance();

    if (draw_animation_first)  // �� ���̃t���O�ŏ�����؂�ւ���
    {
        DrawScrollBackground(); // �w�i & �w�ʃO���b�h�i���j

        // �O���b�h�̗��ɕ`�悷��^�C�~���O�F������ or �ė���
        if (boss != nullptr && (!boss->GetGenerate()))
        {
            objm->DrawBoss();
        }

        DrawFrontGrid();  // �O�ʃO���b�h
        em->Draw();       // ��ɃA�j���[�V����

        // �ʏ�퓬���F�����ς� & �ė����Ă��Ȃ� �� �O���b�h�̑O�ɕ`��
        if (boss != nullptr && boss->GetGenerate())
        {
            objm->DrawBoss();
        }

        // �I�u�W�F�N�g�`��̑O�ɕK���u�����h���[�h��߂�
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        objm->DrawWithoutBoss();      // �v���C���[�E�U�R�Ȃ�
    }
    else
    {
        // �O���b�h�̗��ɕ`�悷��^�C�~���O�F������ or �ė���
        if (boss != nullptr && (!boss->GetGenerate()))
        {
            objm->DrawBoss();
        }

        DrawScrollBackground(); // �w�i & �w�ʃO���b�h�i���j

        DrawFrontGrid();  // �O�ʃO���b�h

        // �ʏ�퓬���F�����ς� & �ė����Ă��Ȃ� �� �O���b�h�̑O�ɕ`��
        if (boss != nullptr && boss->GetGenerate())
        {
            objm->DrawBoss();
        }
        // �I�u�W�F�N�g�`��̑O�ɕK���u�����h���[�h��߂�
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        objm->DrawWithoutBoss();      // �v���C���[�E�U�R�Ȃ�
        em->Draw();       // ��ɃA�j���[�V����
    }




    //// -------- �X�e�[�W���o�FNeural Grid --------
    //if (stage_timer < 5.0f)
    //{
    //    int y_top = (360 - 100);  // �Œ�l�ł�OK
    //    int y_bottom = (360 + 100);

    //    // �p���X���C�e�B���O�i���Ń��C���j
    //    float pulse = (sinf(stage_timer * 6.0f) + 1.0f) * 0.5f; // 0.0?1.0
    //    int pulse_alpha = static_cast<int>(pulse * 180);

    //    SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse_alpha);
    //    DrawLine(0, y_top, 1280, y_top, GetColor(120, 220, 255));
    //    DrawLine(0, y_bottom, 1280, y_bottom, GetColor(120, 220, 255));
    //    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    //    // �O���b�`���̃����_���X�L�������C���i��������o�j
    //    for (int i = 0; i < 8; ++i)
    //    {
    //        int glitch_y = y_top + rand() % (2 * 100);
    //        int glitch_len = 50 + rand() % 100;
    //        int glitch_x = rand() % (1280 - glitch_len);

    //        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
    //        DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
    //        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    //    }

    //    int center_x = D_WIN_MAX_X / 2;

    //    float slide_in = 1.0f;  // ���\���ŃA�j�����Ȃ��Ȃ�Œ�l�ł�
    //    float t = slide_in;

    //    int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
    //    int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

    //    SetFontSize(44);
    //    DrawString(stage_name_x, 320,
    //        "Neural Grid", GetColor(255, 255, 255));
    //    SetFontSize(22);
    //    DrawString(sub_text_x, 370,
    //        "Eliminate all hostile units.", GetColor(120, 255, 255));
    //    SetFontSize(16);
    //}

    DrawGlitchEffect(); // �� �őO�ʂɃO���b�`�\��

    // �N���A���̉��o
    if (is_clear)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "NEXT STAGE", GetColor(255, 255, 255), TRUE);
        SetFontSize(16);
    }
    // �Q�[���I�[�o�[���̉��o
    else if (is_over)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME OVER", GetColor(255, 255, 255), TRUE);
        SetFontSize(16);
    }
}

// ���̃X�e�[�W���擾
StageBase* Stage3::GetNextStage(Player* player)
{
    return new Stage4(player);
}

// �w�i�X�N���[���̕`��
void Stage3::DrawScrollBackground() const
{
    // === ���̃O���b�h�`�� ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(100, 0, 100));

    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back) {
        int sy = y - static_cast<int>(scroll_back) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(100, 0, 100));
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);

    // === ���q�`�� ===
    for (const auto& p : star_particles)
    {
        int a = static_cast<int>(p.alpha);
        if (a <= 0) continue;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);

        DrawLine(static_cast<int>(p.pos.x), static_cast<int>(p.pos.y),
            static_cast<int>(p.pos.x), static_cast<int>(p.pos.y + p.length),
            GetColor(180, 130, 255));
    }

    // === �m�C�Y�G�t�F�N�g ===
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    for (int i = 0; i < 5; ++i) {
        if (rand() % 60 == 0) {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;
            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(255, 100, 50), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// �w�i�X�N���[���̍X�V����
void Stage3::ScrollEffectUpdate(float delta_second)
{
    scroll_back -= delta_second * 220.2f;   // ���O���b�h�͒x��
    scroll_front -= delta_second * 620.0f;  // �O�ʃO���b�h�͑���

    // ���q�̍X�V
    for (auto& p : star_particles)
    {
        p.pos.y += p.velocity.y * delta_second;
        p.alpha -= delta_second * 30.0f;
    }

    star_particles.erase(
        std::remove_if(star_particles.begin(), star_particles.end(),
            [](const StarParticle& p)
            { return (p.pos.y > D_WIN_MAX_Y || p.alpha <= 0); }),
        star_particles.end());

    // ���q�ǉ��i���׌y���̂��ߖ��t���[���ǉ����Ȃ��j
    if (star_particles.size() < 60)
    {
        StarParticle p;
        p.pos = Vector2D(GetRand(D_WIN_MAX_X), GetRand(D_WIN_MAX_Y));
        p.velocity = Vector2D(0, 60.0f + GetRand(30));
        p.alpha = 100.0f + GetRand(100);
        p.length = 10.0f + GetRand(10);
        p.life = 2.0f + GetRand(100) / 50.0f;
        star_particles.push_back(p);
    }
}

void Stage3::DrawFrontGrid() const
{
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(200, 40, 200), TRUE);

    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front) {
        int sy = y - static_cast<int>(scroll_front) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(200, 40, 200), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// �G�̏o�����X�e�[�W�^�C�}�[�ŊǗ�
void Stage3::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    stage_timer += delta_second;
    enemy_spawn_timer += delta_second;
    enemy3_spawn_timer += delta_second;

    // 1. �{�X���o�Ă����Zako�͏o�����Ȃ�
    if (boss_spawned) return;

    if (stage_timer < 4.5f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const int num = 6;
        const float spacing = 100.0f;
        const float appear_y = -100.0f;

        float spawn_interval = my_max(2.0f - stage_timer / 5.0f, 0.5f);
        if (!spawning_zako_left && enemy_spawn_timer >= spawn_interval)
        {
            spawn_index_left = 0;
            spawn_delay_timer_left = 0.0f;
            spawning_zako_left = true;
            enemy_spawn_timer = 0.0f;
        }

        if (!spawning_zako_left) return;

        spawn_delay_timer_left -= delta_second;
        if (spawn_delay_timer_left > 0.0f) return;

        float base_x = 100.0f;
        float dx = spacing;
        float end_base_x = D_WIN_MAX_X / 2 - 180.0f;
        float end_dx = spacing;
        float target_y = 200.0f;

        Vector2D appear_pos(base_x + dx * spawn_index_left, appear_y);
        Vector2D end_pos(end_base_x + end_dx * spawn_index_left, target_y);
        float delay = spawn_index_left * 0.5f;

        auto zako = objm->CreateObject<Enemy3>(appear_pos);
        zako->SetMode(ZakoMode::Zako3);
        zako->SetAppearParams(appear_pos, end_pos, 1.3f + delay, true); // ������
        zako->SetPlayer(player);

        spawn_index_left++;
        if (spawn_index_left >= num)
        {
            spawning_zako_left = false;
        }
        else
        {
            spawn_delay_timer_left = 0.2f;
        }
    }
    else if (stage_timer < 6.8f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const int num = 6;
        const float spacing = 100.0f;
        const float appear_y = -100.0f;

        float spawn_interval = my_max(2.0f - stage_timer / 5.0f, 0.5f);
        if (!spawning_zako_right && enemy_spawn_timer >= spawn_interval)
        {
            spawn_index_right = 0;
            spawn_delay_timer_right = 0.0f;
            spawning_zako_right = true;
            enemy_spawn_timer = 0.0f;
        }

        if (!spawning_zako_right) return;

        spawn_delay_timer_right -= delta_second;
        if (spawn_delay_timer_right > 0.0f) return;

        float base_x = D_WIN_MAX_X - 100.0f;
        float dx = -spacing;
        float end_base_x = D_WIN_MAX_X / 2 + 120.0f;
        float end_dx = -spacing;
        float target_y = 300.0f;

        Vector2D appear_pos(base_x + dx * spawn_index_right, appear_y);
        Vector2D end_pos(end_base_x + end_dx * spawn_index_right, target_y);
        float delay = spawn_index_right * 0.5f;

        auto zako = objm->CreateObject<Enemy3>(appear_pos);
        zako->SetMode(ZakoMode::Zako3);
        zako->SetAppearParams(appear_pos, end_pos, 1.3f + delay, false); // �E����
        zako->SetPlayer(player);

        spawn_index_right++;
        if (spawn_index_right >= num)
        {
            spawning_zako_right = false;
        }
        else
        {
            spawn_delay_timer_right = 0.2f;
        }
    }
    else if (stage_timer < 15.0f)
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        const float spawn_interval = 4.0f;
        if (enemy3_spawn_timer < spawn_interval) return;

        const int num = 6;
        const float spacing = 80.0f;
        const float delay_per = 0.3f;

        float start_x = D_WIN_MAX_X / 2 - spacing;
        float start_y = -100.0f;  // ���ォ��o���������̂ŉ�ʏ�̊O
        float target_start_x = D_WIN_MAX_X / 2 - spacing;
        float target_start_y = 0.0f; // ����ʒu��Y�̊J�n�l

        for (int i = 0; i < num; ++i)
        {
            float x = start_x + spacing * i;
            float y = start_y + spacing * i;  // �� Y���i�K�I�ɑ������ĊK�i��
            float tx = target_start_x + spacing * i;
            float ty = target_start_y + spacing * i;

            float delay = 1.0f + delay_per * i;

            Vector2D appear_pos(x, y);
            Vector2D target_pos(tx, ty);

            auto zako = objm->CreateObject<Enemy3>(appear_pos);
            zako->SetMode(ZakoMode::Zako2);
            zako->SetAppearParams(appear_pos, target_pos, delay, true);
            zako->SetPlayer(player);
        }

        enemy3_spawn_timer = 0.0f;
    }
    else if (stage_timer < 20.0f)
    {
    }
    else if (stage_timer < 25.0f)
    {
        if (!is_enemy3_group_spawned)
        {
            const float base_y = -50.0f;
            const float center_x = D_WIN_MAX_X / 2;
            const float center_y = D_WIN_MAX_Y / 2 - 30.0f;  // �����ォ��n�߂�

            const float step_x = 70.0f;   // �������̍L����
            const float step_y = 90.0f;   // �c�����̒i���i�傫�߁j

            const float delay_step = 0.3f;

            // �����iX: �������獶�ցAY: �と���j
            for (int i = 0; i < 3; ++i)
            {
                float x = center_x - step_x * (i + 1);         // �������獶�ɗ���Ă���
                float y = center_y + step_y * i;               // �ォ�牺�֒i��
                float delay = 1.0f + delay_step * i;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, base_y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, base_y), Vector2D(x, y), delay, true);
                zako->SetPlayer(player);
            }

            // �E���iX: ��������E�ցAY: �と���j
            for (int i = 0; i < 3; ++i)
            {
                float x = center_x + step_x * (i + 1);         // ��������E�ɗ���Ă���
                float y = center_y + step_y * i;               // �ォ�牺�֒i��
                float delay = 1.0f + delay_step * i;

                auto zako = objm->CreateObject<Enemy3>(Vector2D(x, base_y));
                zako->SetMode(ZakoMode::Zako7);
                zako->SetAppearParams(Vector2D(x, base_y), Vector2D(x, y), delay, false);
                zako->SetPlayer(player);
            }
            is_enemy3_group_spawned = true;
        }

    }
    else if (stage_timer < 34.5f)
    {
        //is_warning = true;
    }
    else
    {
        GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

        if (boss_spawned) return;

        objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2 + 60, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss3>(Vector2D(670, -200));
        boss->SetPlayer(player);
        //enemy_list.push_back(boss2);
        boss_spawned = true;
        //is_warning = false;
    }
}

// �N���A����
void Stage3::UpdateGameStatus(float delta_second)
{
    // �{�X���|�ꂽ�烌�C���[��ύX����
    if (boss != nullptr && boss->GetIsCrashing() == true)
    {
        draw_animation_first = true;
    }

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

    //// �Q�[���I�[�o�[���������̉��o����
    //if (is_over == true)
    //{
    //    scene_timer += delta_second;
    //    //gameover_timer += delta_second;

    //    //if (gameover_timer >= 0.005f)
    //    //{
    //    //    transparent++;
    //    //    gameover_timer = 0.0f;
    //    //}

    //    if (scene_timer >= 5.0f)
    //    {
    //        is_finished = true;
    //    }
    //}

    // �X�e�[�W�I�����̓���
    if (is_clear == true || is_over == true)
    {
        // �����ҋ@������I��
        scene_timer += delta_second;
        trans_timer += delta_second;

        if (trans_timer >= 0.02f)
            if (transparent < 255)
                transparent++;

        if (scene_timer >= 5.0f)
        {
            is_finished = true;
        }
        else
        {
            if (player)
                player->SetShotStop();
        }
    }
}
