#include "Stage4.h"
#include "Stage3.h"
#include "../../../../Object/Character/Boss/Boss4.h"
#include "../../../../Object/Item/Shield/Shield.h"
#include "../../../../Object/Item/PowerUp/PowerUp.h"

Stage4::Stage4(Player* player)
    : StageBase(player)
{
}

Stage4::~Stage4()
{
}

void Stage4::Initialize()
{
    stage_id = StageID::Stage4;
}

void Stage4::Finalize()
{
}

void Stage4::Update(float delta_second)
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
}

void Stage4::Draw()
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
    if (stage_timer < 5.0f)
    {
        int y_top = (360 - 100);  // �Œ�l�ł�OK
        int y_bottom = (360 + 100);

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
            int glitch_y = y_top + rand() % (2 * 100);
            int glitch_len = 50 + rand() % 100;
            int glitch_x = rand() % (1280 - glitch_len);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100 + rand() % 100);
            DrawBox(glitch_x, glitch_y, glitch_x + glitch_len, glitch_y + 2, GetColor(200, 255, 255), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }

        int center_x = D_WIN_MAX_X / 2;

        float slide_in = 1.0f;  // ���\���ŃA�j�����Ȃ��Ȃ�Œ�l�ł�
        float t = slide_in;

        int stage_name_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));
        int sub_text_x = static_cast<int>((1.0f - t) * 1280 + t * (center_x - 150));

        SetFontSize(44);
        DrawString(stage_name_x, 320,
            "Neural Grid", GetColor(255, 255, 255));
        SetFontSize(22);
        DrawString(sub_text_x, 370,
            "Eliminate all hostile units.", GetColor(120, 255, 255));
        SetFontSize(16);
    }

    // �N���A���̉��o
    if (is_clear)
    {
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        SetFontSize(32);
        DrawString(D_WIN_MAX_X / 2 - 100.0f, D_WIN_MAX_Y / 2, "GAME CLEAR", GetColor(255, 255, 255), TRUE);
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
StageBase* Stage4::GetNextStage(Player* player)
{
    return nullptr;
}

// �w�i�X�N���[���̕`��
void Stage4::DrawScrollBackground() const
{
    static float time = 0.0f;
    time += 0.05f;

    // �w�i�F�F���Ԃ݂��������O���[
    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(30, 10, 10), TRUE);

    // === �w�ʃ��C���[�i���E�ׂ����E�Â߁j ===
    const int grid_size_back = 40;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_back)
        DrawLine(x, 0, x, D_WIN_MAX_Y, GetColor(100, 0, 0));

    for (int y = -grid_size_back; y < D_WIN_MAX_Y + grid_size_back; y += grid_size_back) {
        int sy = y - static_cast<int>(bg_scroll_offset_layer1) % grid_size_back;
        DrawLine(0, sy, D_WIN_MAX_X, sy, GetColor(100, 0, 0));
    }

    // === �O�ʃ��C���[�i�����E���邭�E�x�����j ===
    const int grid_size_front = 80;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

    for (int x = 0; x < D_WIN_MAX_X; x += grid_size_front)
        DrawBox(x - 1, 0, x + 1, D_WIN_MAX_Y, GetColor(255, 40, 40), TRUE);

    for (int y = -grid_size_front; y < D_WIN_MAX_Y + grid_size_front; y += grid_size_front) {
        int sy = y - static_cast<int>(bg_scroll_offset_layer2) % grid_size_front;
        DrawBox(0, sy - 1, D_WIN_MAX_X, sy + 1, GetColor(255, 40, 40), TRUE);
    }

    // === �m�C�Y���o�i���󊴁j ===
    for (int i = 0; i < 5; ++i) {
        if (rand() % 70 == 0) {
            int nx = rand() % D_WIN_MAX_X;
            int ny = rand() % D_WIN_MAX_Y;
            DrawBox(nx, ny, nx + 3, ny + 3, GetColor(255, 100, 50), TRUE); // �I�����W�x���h�b�g
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
}

// �X�e�[�W�Ǘ��p�^�C�}�[���g���ēG�𐶐�
void Stage4::EnemyAppearance(float delta_second)
{
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();

    if (boss4_spawned) return;


    if (stage_timer < 10.0f)
    {
    }
    else
    {
        objm->CreateObject<PowerUp>(Vector2D(D_WIN_MAX_X / 2 - 60, 120))->SetPlayer(player);
        objm->CreateObject<Shield>(Vector2D(D_WIN_MAX_X / 2, 120))->SetPlayer(player);

        boss = objm->CreateObject<Boss4>(Vector2D(D_WIN_MAX_X / 2, -100));
        boss->SetPlayer(player);
        boss4_spawned = true;
    }
}

// �N���A����
void Stage4::UpdateGameStatus(float delta_second)
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

        if (trans_timer >= 0.01f)
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
