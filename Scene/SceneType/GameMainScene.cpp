#include "GameMainScene.h"
#include "../../Object/GameObjectManager.h"
#include "../../Utility/EffectManager.h"
#include "../../Utility/SEManager.h"
#include "../../Utility/ScoreData.h"
#include "../../Object/Character/Player/Player.h"
#include "Stage/StageType/Stage1.h"
#include "Stage/StageType/Stage2.h"
#include "Stage/StageType/Stage3.h"
#include "Stage/StageType/Stage4.h"
#include "../../Utility/ShakeManager.h"

GameMainScene::GameMainScene()
{
}

GameMainScene::~GameMainScene()
{
}

// ����������
void GameMainScene::Initialize()
{
    // �I�u�W�F�N�g�Ǘ��N���X�̃C���X�^���X���擾
    GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
    player = objm->CreateObject<Player>(Vector2D(D_WIN_MAX_X / 2, (D_WIN_MAX_Y / 2) + 220.0f));

    current_stage = new Stage1(player);

    current_stage->Initialize();

    // BGM�ǂݍ��݁i����̂݁j
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    stage_bgm1 = rm->GetSounds("Resource/sound/bgm/stage/Magical World.mp3");
    stage_bgm3 = rm->GetSounds("Resource/sound/bgm/stage/Cybernetic.mp3");

   // stage_bgm4 = rm->GetSounds("Resource/sound/bgm/stage/Last_Boss.mp3"); // �C�ӂ̃t�@�C��
    stage_bgm4 = rm->GetSounds("Resource/sound/bgm/stage/Boss4_BGM.mp3"); // �C�ӂ̃t�@�C��
    se_warning = rm->GetSounds("Resource/sound/se/battle/Warning.mp3");       // �x����SE

    se_charge = rm->GetSounds("Resource/sound/se/effect/audiostock_1133382.mp3");
    ChangeVolumeSoundMem(255 * 100 / 60, se_charge);

    cursor_se = rm->GetSounds("Resource/sound/se/se_effect/cursor.mp3");

    //�T�C�h�p�l���摜
    obi_handle = rm->GetImages("Resource/Image/BackGround/Main/Obi_1.png")[0];

    //�t�H���g
    font_digital = CreateFontToHandle("Orbitron", 28, 6, DX_FONTTYPE_ANTIALIASING);
    font_orbitron = CreateFontToHandle("Orbitron", 22, 6, DX_FONTTYPE_ANTIALIASING);
    font_warning = CreateFontToHandle("Orbitron", 20, 6, DX_FONTTYPE_ANTIALIASING);

    m_menuFontHandle = CreateFontToHandle("Orbitron", 36, 6); // ���j���[��p�t�H���g

    // �X�e�[�W1�pBGM���Đ�
    current_bgm_handle = stage_bgm1;

    ChangeVolumeSoundMem(255 * 100 / 60, current_bgm_handle);

    PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);

    //AnimationManager �ɂ�鎖�O�A�j���Đ��i0.01�b�A��ʊO�ň�x�����Đ��j
    EffectManager* effe = Singleton<EffectManager>::GetInstance();
    SEManager* se = Singleton<SEManager>::GetInstance();

    effe->LoadAllEffects(); // �������Ŗ����I�ɉ摜���ǂ݂�����I
    se->LoadSE();

    previous_score_count = 0.0f;
    score_logs.clear(); // �O�̂��߃��Z�b�g
}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="delta_second">�P�t���[��������̎���</param>
/// <returns></returns>
eSceneType GameMainScene::Update(float delta_second)
{
    // ���̓C���X�^���X�̎擾
    InputManager* input = Singleton<InputManager>::GetInstance();

    // �|�[�Y�@�\
    if (input->GetButtonDown(XINPUT_BUTTON_START) ||
        input->GetKeyDown(KEY_INPUT_P)) 
    {
        isPaused = !isPaused;
        m_selectedIndex = 0;
    }

    // ======= �X�R�A���O�̃X���C�h���o�X�V =======
    for (auto& log : score_logs)
    {
        if (log.y_offset < 0.0f)
        {
            // �C�[�W���O���Ɋ��炩�ɕ␳�i���x���ጸ���j
            log.y_offset += (-log.y_offset) * 10.0f * delta_second;

            // �덷�z���i0�ɋ߂Â�����0�ɂ���j
            if (log.y_offset > -0.5f)
                log.y_offset = 0.0f;
        }
    }

    // ======= �V�����X�R�A�̒ǉ����o�ƃ��O���� =======
    ScoreData* score = Singleton<ScoreData>::GetInstance();
    const auto& all_scores = score->GetScoreData();

    while (previous_score_count < all_scores.size())
    {
        float new_score = all_scores[static_cast<size_t>(previous_score_count)];

        // �X�R�A���O�p���b�Z�[�W�쐬
        char buf[64];
        sprintf_s(buf, sizeof(buf), "Score +%.0f", new_score);

        // �� ��ɍő�s���`�F�b�N�i10�s��������Â��̂���폜�j
        if (score_logs.size() >= 10)
        {
            score_logs.erase(score_logs.begin()); // ��ԌÂ����O���폜
        }

        // �������O�����ɉ����o��
        for (auto& log : score_logs)
        {
            log.y_offset = -20.0f; // �X���C�h�J�n�ʒu
        }

        // �V�������O��ǉ��i�X���C�h�Ȃ��j
        ScoreLog new_log;
        new_log.text = buf;
        new_log.y_offset = 0.0f;
        score_logs.push_back(new_log);

        previous_score_count += 1.0f;
    }

    if (player)
    {
        // �V�[���hON���o
        static bool prev_shield = false;
        bool now_shield = player->GetShieldOn();
        if (!prev_shield && now_shield) {
            effect_shield_on = true;
            effect_timer = 0.0f;
        }
        if (prev_shield && !now_shield) {
            effect_shield_off = true;
            effect_timer = 0.0f;
        }
        prev_shield = now_shield;

        // �p���[�A�b�v���o�i1�t���[���̂݁j
        static int prev_power = 1;
        int now_power = player->GetPowerd();
        if (now_power > prev_power) {
            effect_powerup = true;
            effect_timer = 0.0f;
        }
        prev_power = now_power;
    }

    // �^�C�}�[�i�s
    effect_timer += delta_second;
    if (effect_timer > effect_duration) {
        effect_shield_on = effect_shield_off = effect_powerup = false;
    }
    
    if (player)
        if (player->GetNowType() == PlayerType::AlphaCode)
            StartUITransition(PlayerType::AlphaCode);
        else
            StartUITransition(PlayerType::OmegaCode);

    if(player)
        UpdateUITransition(delta_second);


    if (!isPaused) {
        eSceneType type = UpdateGameplay(delta_second);
        return type;
    }
    else {
        eSceneType type = UpdatePauseMenu(delta_second);
        return type;
    }

	return GetNowSceneType();
}

/// <summary>
/// �`�揈��
/// </summary>
/// <returns></returns>
void GameMainScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "GameMainScene", GetColor(255, 255, 255));
#endif
    if (current_stage)
    {
        EffectManager* anim = Singleton<EffectManager>::GetInstance();
        anim->Draw();
        current_stage->Draw();
        DrawUI();

        if (ui_transitioning)  // UI�؂�ւ����̂ݕ`��
        {
            DrawLineNoiseEffectForSideUI(0.7f); // �� �D�݂ŋ��x����
        }

        if (current_stage->GetStageID() == StageID::Stage3)
        {
            if (current_stage->IsClear())
            {
                if (black_fade_timer >= 0.01f)
                {
                    black_fade_timer = 0.0f;
                    alpha += 2;
                }
                //int alpha = static_cast<int>((black_fade_timer / black_fade_duration) * 255.0f);
                if (alpha > 255) alpha = 255;

                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                if (alpha >= 255)
                {
                    current_stage->SetFinished();
                }
            }
        }
        else if (current_stage->GetStageID() == StageID::Stage4)
        {
            if (black_in_timer2 >= 8.0f)
            {
                if (black_in_timer >= 0.01f)
                {
                    black_in_timer = 0.0f;
                    alpha--;
                }
            }

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE); // �� �w�i���ŏ��ɕ`��
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // �� �����ǉ��I�i�����d�v�I�j ?

            if (warning_state != WarningState::None)
            {
                float flash_alpha = 200.0f * sinf(warning_timer * 10.0f);
                if (flash_alpha < 0) flash_alpha = 0;

                // ===== �x���I���ԍۂɑтƕ������t�F�[�h�A�E�g =====
                float fade_start_time = warning_duration - 1.0f;
                int band_alpha = 255;
                if (warning_timer >= fade_start_time)
                {
                    float t = (warning_duration - warning_timer);
                    if (t < 0.0f) t = 0.0f;
                    band_alpha = static_cast<int>(255 * t);  // 0?255�Ɍ���
                }

                // ===== �w�i�t���b�V���i�ԁj =====
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)flash_alpha);
                DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 50, 50), TRUE);

                SetDrawBlendMode(DX_BLENDMODE_ALPHA, band_alpha);

                const int band_height = 120;
                const char* warn_text = "!! WARNING !!";
                const char* scroll_text = "!! SECURITY WARNING  FINAL PROTOCOL ACTIVE  SYSTEM LOCKDOWN ENGAGED !! - !! SECURITY WARNING  FINAL PROTOCOL ACTIVE  SYSTEM LOCKDOWN ENGAGED !!";
                int scroll_speed = 200;

                int text_width = GetDrawStringWidthToHandle(scroll_text, strlen(scroll_text), font_orbitron);
                int scroll_color = GetColor(255, 100 + GetRand(80), 80);

                float scale = 3.0f;
                int base_w = GetDrawStringWidthToHandle(warn_text, strlen(warn_text), font_warning);
                int draw_w = (int)(base_w * scale);
                int draw_h = (int)(32 * scale);

                int x = (D_WIN_MAX_X - draw_w) / 2;
                int y = (D_WIN_MAX_Y - draw_h) / 2;

                int offset_x = GetRand(5) - 2;
                int offset_y = GetRand(3) - 1;

                int pulse = (GetNowCount() % 100 < 50) ? 255 : 100;
                int color = GetColor(255, pulse, pulse);

                // �������� WARNING
                DrawExtendStringToHandle(x + offset_x, y + offset_y, scale, scale, warn_text, color, font_warning);

                // === ��єw�i�ƃ��C�� ===
                DrawBox(0, 0, D_WIN_MAX_X, band_height, GetColor(30, 0, 0), TRUE);
                DrawLine(0, band_height - 2, D_WIN_MAX_X, band_height - 2, GetColor(255, GetRand(100), 100));

                float scroll_scale = 1.8f;
                int scroll_font_h = (int)(32 * scroll_scale);
                int scroll_x_top = static_cast<int>(D_WIN_MAX_X - (warning_timer * scroll_speed));
                if (scroll_x_top + text_width * scroll_scale > 0)
                {
                    DrawExtendStringToHandle(scroll_x_top, band_height / 2 - scroll_font_h / 2,
                        scroll_scale, scroll_scale, scroll_text, scroll_color, font_orbitron);
                }

                // === ���єw�i�ƃ��C�� ===
                DrawBox(0, D_WIN_MAX_Y - band_height, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(30, 0, 0), TRUE);
                DrawLine(0, D_WIN_MAX_Y - band_height + 1, D_WIN_MAX_X, D_WIN_MAX_Y - band_height + 1, GetColor(255, GetRand(100), 100));

                int scroll_x_bot = static_cast<int>(-(text_width * scroll_scale) + (warning_timer * scroll_speed));
                if (scroll_x_bot < D_WIN_MAX_X)
                {
                    DrawExtendStringToHandle(scroll_x_bot, D_WIN_MAX_Y - band_height + band_height / 2 - scroll_font_h / 2,
                        scroll_scale, scroll_scale, scroll_text, scroll_color, font_orbitron);
                }

                // ===== �m�C�Y�G�t�F�N�g�i�я�j =====
                for (int i = 0; i < 15; ++i)
                {
                    int nx = GetRand(D_WIN_MAX_X);
                    int ny = GetRand(band_height);
                    int nl = GetRand(40) + 20;

                    DrawBox(nx, ny, nx + nl, ny + 2, GetColor(100 + GetRand(100), 0, 0), TRUE);
                    DrawBox(nx, D_WIN_MAX_Y - band_height + ny, nx + nl, D_WIN_MAX_Y - band_height + ny + 2, GetColor(100 + GetRand(100), 0, 0), TRUE);
                }

                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }

            if (alpha < 0)
            {
                alpha = 0;
                black_in_timer2 = 0.0f;
            }
        }

        if (end_sequence_started)
        {
            // �t�F�[�h�A�E�g�i���j
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, end_fade_alpha);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            // ���b�Z�[�W�\���i�t�F�[�h�ƍ��킹�ĕ����яオ��j
            if (end_sequence_timer >= 1.0f)
            {
                int alpha = (int)((end_sequence_timer - 1.0f) * 255);
                if (alpha > 255) alpha = 255;
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

                const char* msg = "GAME CLEAR";
                int text_w = GetDrawStringWidthToHandle(msg, strlen(msg), font_digital);
                int x = (D_WIN_MAX_X - text_w) / 2;
                int y = D_WIN_MAX_Y / 2;

                DrawStringToHandle(x, y, msg, GetColor(255, 255, 255), font_digital);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }

        if (isPaused)
        {
            const int cx = D_WIN_MAX_X / 2;
            const int cy = D_WIN_MAX_Y / 2 - 20;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
            DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
            //DrawBox(400, 200, 880, D_WIN_MAX_Y - 200, GetColor(255, 255, 255), FALSE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

            const char* menuItems[] = {
                "START GAME",
                "BACK TITLE",
            };

            for (int i = 0; i < 2; ++i)
            {
                int y = 300 + i * 50;
                int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
                int x = (D_WIN_MAX_X - textWidth) / 2;

                if (i == m_selectedIndex)
                {
                    // =========================
                    // �w�i�n�C���C�g�o�[�i��ʒ[�܂Łj
                    // =========================
                    int barHeight = 40;
                    int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50); // �p���X
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
                    DrawBox(
                        0, y - 5,
                        D_WIN_MAX_X, y + barHeight,
                        GetColor(0, 200, 255), TRUE
                    );

                    // =========================
                    // ���[�̃G�b�W�G�t�F�N�g�i�������j
                    // =========================
                    int edgeWidth = 80;
                    int edgeHeight = 4;
                    int scrollSpeed = 2;
                    int edgeOffset = (GetNowCount() * scrollSpeed) % (D_WIN_MAX_X + edgeWidth * 2);

                    // ������E�֗����i2�\�����ă��[�v�����o���j
                    for (int j = 0; j < 2; ++j)
                    {
                        int edgeX = edgeOffset - edgeWidth * j;
                        SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
                        DrawBox(
                            edgeX, y + 10,
                            edgeX + edgeWidth, y + 10 + edgeHeight,
                            GetColor(200, 255, 255), TRUE
                        );
                        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                    }

                    // =========================
                    // �e�L�X�g�i�A�E�g���C���{�O���[�j
                    // =========================
                    int offsetX = (rand() % 3) - 1;
                    int offsetY = (rand() % 3) - 1;

                    DrawStringToHandle(x + offsetX - 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX + 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX, y + offsetY - 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                    DrawStringToHandle(x + offsetX, y + offsetY + 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);

                    int glow = (int)((sinf(GetNowCount() / 30.0f) + 1.0f) * 127);
                    DrawStringToHandle(x + offsetX, y + offsetY, menuItems[i], GetColor(100 + glow, 255, 255), m_menuFontHandle);
                }
                else
                {
                    DrawStringToHandle(x, y, menuItems[i], GetColor(180, 180, 180), m_menuFontHandle);
                }
            }
        }

        if (player && player->GetGameOver())
        {
            DrawGameOverEffect();

            // GameOver���j���[���o����^�C�~���O�i��F3.5�b��j
            if (gameover_timer >= 10.0f)
            {
                retry = true;

                SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
                DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

                const char* menuItems[] = {
                    "RETRY GAME",
                    "BACK TITLE",
                };

                for (int i = 0; i < 2; ++i)
                {
                    int y = 300 + i * 50;
                    int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
                    int x = (D_WIN_MAX_X - textWidth) / 2;

                    if (i == m_selectedIndex)
                    {
                        // =========================
                        // �w�i�n�C���C�g�o�[�i��ʒ[�܂Łj
                        // =========================
                        int barHeight = 40;
                        int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50); // �p���X
                        SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
                        DrawBox(
                            0, y - 5,
                            D_WIN_MAX_X, y + barHeight,
                            GetColor(0, 200, 255), TRUE
                        );

                        // =========================
                        // ���[�̃G�b�W�G�t�F�N�g�i�������j
                        // =========================
                        int edgeWidth = 80;
                        int edgeHeight = 4;
                        int scrollSpeed = 2;
                        int edgeOffset = (GetNowCount() * scrollSpeed) % (D_WIN_MAX_X + edgeWidth * 2);

                        // ������E�֗����i2�\�����ă��[�v�����o���j
                        for (int j = 0; j < 2; ++j)
                        {
                            int edgeX = edgeOffset - edgeWidth * j;
                            SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
                            DrawBox(
                                edgeX, y + 10,
                                edgeX + edgeWidth, y + 10 + edgeHeight,
                                GetColor(200, 255, 255), TRUE
                            );
                            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                        }

                        // =========================
                        // �e�L�X�g�i�A�E�g���C���{�O���[�j
                        // =========================
                        int offsetX = (rand() % 3) - 1;
                        int offsetY = (rand() % 3) - 1;

                        DrawStringToHandle(x + offsetX - 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                        DrawStringToHandle(x + offsetX + 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                        DrawStringToHandle(x + offsetX, y + offsetY - 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                        DrawStringToHandle(x + offsetX, y + offsetY + 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);

                        int glow = (int)((sinf(GetNowCount() / 30.0f) + 1.0f) * 127);
                        DrawStringToHandle(x + offsetX, y + offsetY, menuItems[i], GetColor(100 + glow, 255, 255), m_menuFontHandle);
                    }
                    else
                    {
                        DrawStringToHandle(x, y, menuItems[i], GetColor(180, 180, 180), m_menuFontHandle);
                    }
                }
            }

            return;
        }
    }

#if _DEBUG
    if(current_stage)
        DrawFormatString(0, 30, GetColor(255, 255, 255), "Stage %d", current_stage->GetStageID());
#endif
}

// �I���������i�g�����C���X�^���X�̍폜�Ƃ��j
void GameMainScene::Finalize()
{
    GameObjectManager::GetInstance()->Finalize();
    //ScoreData::GetInstance()->
    StopSoundMem(current_bgm_handle); // ���݂�BGM���~

    if (font_orbitron != -1) {
        DeleteFontToHandle(font_orbitron);
        font_orbitron = -1;
    }
    if (font_warning != -1) {
        DeleteFontToHandle(font_warning);
        font_warning = -1;
    }
    if (font_digital != -1) {
        DeleteFontToHandle(font_digital);
        font_digital = -1;
    }
    if (m_menuFontHandle != -1) {
        DeleteFontToHandle(m_menuFontHandle);
        m_menuFontHandle = -1;
    }

    // �g�[�^���X�R�A�������L���O�ɃG���g���[
    ScoreData::GetInstance()->ScoreEntrie(ScoreData::GetInstance()->GetTotalScore());
}

/// <summary>
/// ���݂̃V�[�����
/// </summary>
/// <returns>���݂̓��U���g�V�[���ł�</returns>
eSceneType GameMainScene::GetNowSceneType() const
{
	return eSceneType::eGameMain;
}

void GameMainScene::AddScoreLog(const std::string& text)
{
    // �V�������O����ɒǉ��i�X���C�h�����j
    ScoreLog log;
    log.text = text;
    log.y_offset = 0.0f;
    score_logs.push_back(log);

    // �ő�10�s�ێ� �� �Â����̂���폜
    if (score_logs.size() >= 10)
    {
        score_logs.erase(score_logs.begin()); // �擪�i�Â��j���폜
    }

    // �������O�̓X���C�h�J�n�i���ɉ����o�����j
    for (auto& log : score_logs)
    {
        log.y_offset = -20.0f;  // ���t���[���ŕ␳�����
    }
}

void GameMainScene::DrawUI()
{
    Vector2D offset = ShakeManager::GetInstance()->GetOffset();
    // === ���̍��� ===
#if 0
    DrawGraph(0 + (int)offset.x, 0 + (int)offset.x, obi_handle, TRUE);
    DrawTurnGraph(D_WIN_MAX_X - 290 + (int)offset.x, 0 + (int)offset.x, obi_handle, TRUE);
#else
    DrawGraph(0, 0, obi_handle, TRUE);
    DrawTurnGraph(D_WIN_MAX_X - 290, 0, obi_handle, TRUE);
#endif
    // === ���̃T�C�h�p�l���i�T�C�o�[�� / �^�C�v���o�Ή��j ===
    {
        int type_offset_x = GetUIXOffsetLeft();

        int left_x1 = 0 + (int)offset.x + type_offset_x;
        int left_x2 = (D_WIN_MAX_X / 2) - 350 + (int)offset.x + type_offset_x;
        int left_x3 = 0 + type_offset_x;
        int left_x4 = (D_WIN_MAX_X / 2) - 350 + type_offset_x;

        int panel_color = GetTypeColor(10, 10, 30, 30, 10, 10); // �w�i���
        int neon_color = GetTypeColor(0, 255, 255, 255, 80, 80); // �����

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(left_x3, 0, left_x4, D_WIN_MAX_Y, GetColor(10, 10, 30), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        DrawLine(left_x1, 0 + (int)offset.y, left_x2, 0 + (int)offset.y, neon_color);
        DrawLine(left_x1, D_WIN_MAX_Y - 1 + (int)offset.y, left_x2, D_WIN_MAX_Y - 1 + (int)offset.y, neon_color);

        if (current_stage)
        {
            float scanline_speed = (current_stage->GetStageID() == StageID::Stage4) ? 600.0f : 60.0f;

            // �X�L�������C���̐F���^�C�v���
            int color = GetTypeColor(0, 150, 255, 255, 80, 80);

            int scan_y = static_cast<int>(line_effect_timer * scanline_speed) % D_WIN_MAX_Y;
            DrawLine(left_x1, scan_y + (int)offset.y, left_x2, scan_y + (int)offset.y, color);
        }
    }

    // === �E�̃T�C�h�p�l���i�T�C�o�[�� / �^�C�v���o�Ή��j ===
    {
        int type_offset_x = GetUIXOffsetRight();

        int right_x1 = (D_WIN_MAX_X / 2) + 350 + (int)offset.x + type_offset_x;
        int right_x2 = D_WIN_MAX_X + (int)offset.x + type_offset_x;
        int right_x3 = (D_WIN_MAX_X / 2) + 350 + type_offset_x;
        int right_x4 = D_WIN_MAX_X + type_offset_x;

        int panel_color = GetTypeColor(10, 10, 30, 30, 10, 10); // �w�i���
        int neon_color = GetTypeColor(0, 255, 255, 255, 80, 80); // �����

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(right_x3, 0, right_x4, D_WIN_MAX_Y, GetColor(10, 10, 30), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        DrawLine(right_x1, 0 + (int)offset.y, right_x2, 0 + (int)offset.y, neon_color);
        DrawLine(right_x1, D_WIN_MAX_Y - 1 + (int)offset.y, right_x2, D_WIN_MAX_Y - 1 + (int)offset.y, neon_color);

        if (current_stage)
        {
            float scanline_speed = (current_stage->GetStageID() == StageID::Stage4) ? 600.0f : 60.0f;

            int color = GetTypeColor(0, 150, 255, 255, 80, 80); // �X�L�������C���F���
            int scan_y = static_cast<int>(line_effect_timer * scanline_speed) % D_WIN_MAX_Y;
            DrawLine(right_x1, scan_y + (int)offset.y, right_x2, scan_y + (int)offset.y, color);
        }
    }

    // ==== �X�R�A���O�i���� / �^�C�v���o�Ή��j ====
    {
        int type_offset_x = GetUIXOffsetLeft();

        int log_base_x = 30 + (int)offset.x + type_offset_x;
        int log_base_y = D_WIN_MAX_Y - 400 + (int)offset.y;
        int line_height = 40;

        int count = static_cast<int>(score_logs.size());
        for (int i = 0; i < count; ++i)
        {
            const auto& log = score_logs[count - 1 - i];
            int draw_y = log_base_y + static_cast<int>(i * line_height + log.y_offset);

            int line_color = GetTypeColor(0, 255, 255, 255, 80, 80);
            int text_color = GetTypeColor(0, 255, 255, 255, 160, 160);

            DrawLine(log_base_x - 10, draw_y - 2, log_base_x + 210, draw_y - 2, line_color);

            std::string label = "Score ";
            std::string value;

            const char* plus_pos = strchr(log.text.c_str(), '+');
            value = plus_pos ? plus_pos : "";

            int value_width = GetDrawStringWidthToHandle(value.c_str(), (int)value.size(), font_orbitron);
            int value_x = log_base_x + 200 - value_width;

            DrawStringToHandle(log_base_x, draw_y, label.c_str(), text_color, font_orbitron);
            DrawStringToHandle(value_x, draw_y, value.c_str(), text_color, font_orbitron);
        }
    }

    // ==== LIFE - STOCK�i�E�� / �^�C�v���o�Ή��j ====
    if (player)
    {
        int type_offset_x = GetUIXOffsetRight();

        int x = D_WIN_MAX_X - 230 + (int)offset.x + type_offset_x;
        int y = 80 + (int)offset.y;

        int text_color = GetTypeColor(0, 255, 255, 255, 160, 160);
        int line_color = GetTypeColor(0, 255, 255, 255, 80, 80);
        int icon_color = GetTypeColor(255, 100, 100, 0, 255, 255);

        DrawStringToHandle(x + 10, y + 4, "LIFE - STOCK", text_color, font_orbitron);

        for (int i = 0; i < player->GetLife(); ++i)
        {
            int px = x + 20 + i * 20;
            int py = y + 32;
            int sz = 14;
            DrawTriangle(px, py + sz, px + sz / 2, py, px + sz, py + sz, icon_color, TRUE);
        }

        DrawLine(x, y + 64, x + 200, y + 64, line_color);
    }

    // ==== CHARGE �Q�[�W�i�^�C�v���o�Ή��j ====
    if (player)
    {
        int type_offset_x = GetUIXOffsetRight();

        int x = D_WIN_MAX_X - 230 + (int)offset.x + type_offset_x;
        int y = 150 + (int)offset.y;

        int text_color = GetTypeColor(0, 255, 255, 255, 160, 160);
        int line_color = GetTypeColor(0, 255, 255, 255, 80, 80);
        int base_bar_color = GetTypeColor(30, 30, 30, 40, 10, 10);
        int flash = (GetNowCount() % 100 > 50) ? 255 : 100;

        DrawStringToHandle(x + 10, y + 2, "CHARGE", text_color, font_orbitron);

        float rate = Clamp(player->GetChargeRate(), 0.0f, 1.0f);
        int bar_x = x + 10, bar_y = y + 25;
        int bar_w = 180, bar_h = 12;

        int fill_color = player->CanUseSpecial()
            ? GetTypeColor(0, flash, 255, flash, 80, 80)
            : GetTypeColor(0, 255, 255, 255, 160, 160);

        DrawBox(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, base_bar_color, TRUE);
        DrawBox(bar_x, bar_y, bar_x + static_cast<int>(bar_w * rate), bar_y + bar_h, fill_color, TRUE);

        if (player->CanUseSpecial() && !player->GetGameOver())
        {
            int text_x = x + 140;
            int text_y = y + 30;
            int pulse = (GetNowCount() % 100 > 50) ? 255 : 100;
            int pulse_color = GetTypeColor(255, pulse, pulse, pulse, 80, 80);

            DrawStringToHandle(text_x - 25, text_y + 5, "Press B!!", pulse_color, font_orbitron);
        }

        DrawLine(x, y + 60, x + 200, y + 60, line_color);
    }

    // ==== POWER�i�Q�[�W�^ / �^�C�v���o�Ή��j ====
    if (player)
    {
        int type_offset_x = GetUIXOffsetRight();

        int x = D_WIN_MAX_X - 230 + (int)offset.x + type_offset_x;
        int y = 220 + (int)offset.y;

        int text_color = GetTypeColor(0, 255, 255, 255, 160, 160);
        int line_color = GetTypeColor(255, 255, 255, 255, 100, 100);

        DrawStringToHandle(x + 10, y + 2, "POWER", text_color, font_orbitron);

        int level = player->GetPowerd();
        const int max_level = 3;

        for (int i = 0; i < max_level; ++i)
        {
            int px = x + 20 + i * 20;
            int py = y + 25;

            int color;
            if (i < level)
            {
                color = (level == max_level)
                    ? GetTypeColor(100, 255, 100, 255, 120, 120)
                    : GetTypeColor(255, 255, 100, 255, 180, 120);
            }
            else
            {
                color = GetTypeColor(50, 50, 50, 80, 20, 20);
            }

            DrawBox(px, py, px + 14, py + 14, color, TRUE);
        }

        int bar_start = x + 20;
        int bar_end = x + 20 + (max_level - 1) * 20 + 14;
        int bar_y = y + 42;

        DrawLine(bar_start, bar_y, bar_end, bar_y, line_color);

        DrawStringToHandle(bar_start - 10, bar_y + 5, "E", line_color, font_orbitron);
        DrawStringToHandle(bar_end + 5, bar_y + 5, "F", line_color, font_orbitron);
    }

    // ==== SHIELD�i�^�C�v���o�Ή��j ====
    if (player)
    {
        int type_offset_x = GetUIXOffsetRight();

        int x = D_WIN_MAX_X - 230 + (int)offset.x + type_offset_x;
        int y = 290 + (int)offset.y;

        int text_color = GetTypeColor(0, 255, 255, 255, 160, 160);
        int line_color = GetTypeColor(0, 255, 255, 255, 80, 80);

        DrawStringToHandle(x + 10, y + 2, "SHIELD", text_color, font_orbitron);

        const char* shield_text = player->GetShieldOn() ? "ON" : "OFF";
        int status_color = player->GetShieldOn()
            ? GetTypeColor(0, 255, 100, 100, 255, 200)
            : GetTypeColor(255, 100, 100, 255, 60, 60);

        DrawStringToHandle(x + 20, y + 25, shield_text, status_color, font_digital);

        if (player->GetShieldOn())
        {
            float r = sinf(GetNowCount() / 30.0f) * 5 + 15;
            int circle_color = GetTypeColor(0, 255, 180, 180, 80, 80);
            DrawCircle(x + 150, y + 35, static_cast<int>(r), circle_color, FALSE);
        }

        DrawLine(x, y + 70, x + 200, y + 70, line_color);
    }

    // ==== SPECIAL READY UI�i�v���C���[���ɐ^�񒆕\���j ====
    if (player && player->CanUseSpecial())
    {
        Vector2D pos = player->GetLocation();
        int ui_x = static_cast<int>(pos.x + offset.x) - 55;
        int ui_y = static_cast<int>(pos.y + offset.y) + 40;
        int pulse = static_cast<int>(GetNowCount() % 100) > 50 ? 255 : 100;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawFormatStringToHandle(ui_x, ui_y, GetColor(255, pulse, pulse), font_orbitron, "Press B!!");
    }

    // ==== �T�C�h�p�l���Ƃ̋��E�Ɂu����郉�C���v�G�t�F�N�g�iStage4�͐�?���̃`�J�`�J�j ====
    {
        const int line_width = 4;
        const int flow_speed = 1000;
        const int gap = 70;

        int offset = static_cast<int>(line_effect_timer * flow_speed) % gap;

        int color = GetTypeColor(0, 255, 255, 255, 100, 100);

        // Stage4����
        bool is_stage4 = (current_stage && current_stage->GetStageID() == StageID::Stage4);

        int pulse_timer = (GetNowCount() / 10) % 2;  // ���݂ɓ_�Łi10�t���[�����Ɓj
        int line_color = is_stage4
            ? (pulse_timer == 0 ? GetColor(255, 80, 50) : GetColor(255, 200, 50)) // �ԂƉ��F
            : GetColor(0, 180, 255); // �ʏ�F�l�I���u���[

        int flare_color = is_stage4
            ? (pulse_timer == 0 ? GetColor(255, 120, 120) : GetColor(255, 240, 120)) // �t���A���؂�ւ�
            : GetColor(100, 255, 255);

        int left_x = (D_WIN_MAX_X / 2) - 350;
        int right_x = (D_WIN_MAX_X / 2) + 350 - line_width;

        for (int y = -gap; y < D_WIN_MAX_Y + gap; y += gap)
        {
            int y0 = y + offset;

            // �����C��
            DrawBox(left_x, y0, left_x + line_width, y0 + 20, color, TRUE);
            if (is_stage4)
            {
                DrawCircle(left_x + line_width / 2, y0 + 10, 3, color, TRUE); // �t���A�ǉ�
            }

            // �E���C��
            DrawBox(right_x, y0, right_x + line_width, y0 + 20, color, TRUE);
            if (is_stage4)
            {
                DrawCircle(right_x + line_width / 2, y0 + 10, 3, color, TRUE);
            }
        }
    }

    // ==== �g�[�^���X�R�A�i����j ====
    {
        ScoreData* score = Singleton<ScoreData>::GetInstance();
        float total_score = score->GetTotalScore();

        int x = 30 + (int)offset.x + GetUIXOffsetLeft(); // �� �C����
        int y = 80 + (int)offset.y;
        int w = 240, h = 80;
        int line_color = GetTypeColor(0, 255, 255, 255, 80, 80);
        int text_color = GetTypeColor(0, 255, 255, 255, 80, 80);


        DrawLine(x, y, x + w, y, line_color);
        DrawLine(x, y + h, x + w, y + h, line_color);

        DrawStringToHandle(x + 10, y + 8, "TOTAL SCORE", text_color, font_orbitron);

        char score_str[64];
        sprintf_s(score_str, sizeof(score_str), "%.0f", total_score);

        int score_width = GetDrawStringWidthToHandle(score_str, strlen(score_str), font_digital);
        int score_x = x + w - 20 - score_width;

        DrawStringToHandle(score_x, y + 40, score_str, GetColor(255, 255, 100), font_digital);
    }

    // ==== ����UI�i�E���j ====
    {
        const int base_x = D_WIN_MAX_X - 250 + (int)offset.x + GetUIXOffsetRight(); // �� �C����
        const int base_y = D_WIN_MAX_Y - 240 + (int)offset.y;
        const int line_height = 42;

        int text_color = GetTypeColor(0, 255, 255, 255, 80, 80);

        const char* inputs[] = {
            "MOVE    : LeftStick",
            "SHOT    : A",
            "SPECIAL : B",
            "PAUSE   : Start"
        };

        int pulse = (GetNowCount() % 100 < 50) ? 255 : 100;
        int op_color = GetColor(100, pulse, 255);

        for (int i = 0; i < 4; ++i)
        {
            int y = base_y + i * line_height;
            DrawStringToHandle(base_x, y, inputs[i], text_color, font_orbitron);
        }
    }

    if (isPaused)
    {
        const int cx = D_WIN_MAX_X / 2;
        const int cy = D_WIN_MAX_Y / 2 - 20;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, transparent);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
        //DrawBox(400, 200, 880, D_WIN_MAX_Y - 200, GetColor(255, 255, 255), FALSE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


        const char* menuItems[] = {
            "START GAME",
            "BACK TITLE",
        };

        for (int i = 0; i < 2; ++i)
        {
            int y = 300 + i * 50;
            int textWidth = GetDrawStringWidthToHandle(menuItems[i], strlen(menuItems[i]), m_menuFontHandle);
            int x = (D_WIN_MAX_X - textWidth) / 2;

            if (i == m_selectedIndex)
            {
                // =========================
                // �w�i�n�C���C�g�o�[�i��ʒ[�܂Łj
                // =========================
                int barHeight = 40;
                int barAlpha = 120 + (int)(sinf(GetNowCount() / 60.0f) * 50); // �p���X
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, barAlpha);
                DrawBox(
                    0, y - 5,
                    D_WIN_MAX_X, y + barHeight,
                    GetColor(0, 200, 255), TRUE
                );

                // =========================
                // ���[�̃G�b�W�G�t�F�N�g�i�������j
                // =========================
                int edgeWidth = 80;
                int edgeHeight = 4;
                int scrollSpeed = 2;
                int edgeOffset = (GetNowCount() * scrollSpeed) % (D_WIN_MAX_X + edgeWidth * 2);

                // ������E�֗����i2�\�����ă��[�v�����o���j
                for (int j = 0; j < 2; ++j)
                {
                    int edgeX = edgeOffset - edgeWidth * j;
                    SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
                    DrawBox(
                        edgeX, y + 10,
                        edgeX + edgeWidth, y + 10 + edgeHeight,
                        GetColor(200, 255, 255), TRUE
                    );
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                }

                // =========================
                // �e�L�X�g�i�A�E�g���C���{�O���[�j
                // =========================
                int offsetX = (rand() % 3) - 1;
                int offsetY = (rand() % 3) - 1;

                DrawStringToHandle(x + offsetX - 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX + 1, y + offsetY, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX, y + offsetY - 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);
                DrawStringToHandle(x + offsetX, y + offsetY + 1, menuItems[i], GetColor(0, 0, 0), m_menuFontHandle);

                int glow = (int)((sinf(GetNowCount() / 30.0f) + 1.0f) * 127);
                DrawStringToHandle(x + offsetX, y + offsetY, menuItems[i], GetColor(100 + glow, 255, 255), m_menuFontHandle);
            }
            else
            {
                DrawStringToHandle(x, y, menuItems[i], GetColor(180, 180, 180), m_menuFontHandle);
            }
        }
    }

    if (effect_shield_on || effect_shield_off || effect_powerup)
    {
        int alpha = static_cast<int>(100 * (1.0f - (effect_timer / effect_duration))); // �ő�100�ɗ}����

        int color = GetColor(0, 0, 0); // �f�t�H���g�i�������j

        if (effect_shield_on)
            color = GetColor(0, 255, 180); // �Όn�i�V�[���hON�j
        else if (effect_shield_off)
            color = GetColor(255, 50, 50); // �Ԍn�i�V�[���h�j��j
        else if (effect_powerup)
            color = GetColor(255, 255, 100); // ���n�i�p���[�A�b�v�j

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, color, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

// �x�����o�̍X�V����
void GameMainScene::WarningUpdate(float delta_second)
{
    if (warning_state == WarningState::None && is_none == false)
    {
        // �����ɉ����āi��F��莞�Ԍo�ߌ�Ȃǁj
        if (black_in_timer2 >= 2.0f)
        {
            is_none = true;
            warning_state = WarningState::Expanding;
            warning_timer = 0.0f;
            band_half_height = 0.0f; // �O�̂��ߏ�����
            warning_text_x = 1020;   // �����ʒu�������i�E�[���痬���ꍇ�j

            if (player)
                player->SetShotStop(true);
        }

    }

    black_in_timer2 += delta_second;
    warning_timer += delta_second;

    switch (warning_state)
    {
    case WarningState::Expanding:
        band_half_height += band_expand_speed * delta_second;
        if (band_half_height >= band_max_half_height)
        {
            band_half_height = band_max_half_height;
            warning_state = WarningState::Displaying;
            warning_timer = 0.0f;

            // �x��SE�Đ�
            PlaySoundMem(se_warning, DX_PLAYTYPE_BACK);
        }
        break;

    case WarningState::Displaying:
        warning_text_x -= warning_scroll_speed * delta_second;
        warning_timer += delta_second;

        if (warning_text_x < -200)
            warning_text_x = 1020;

        if (warning_timer >= warning_duration)
        {
            warning_state = WarningState::Shrinking;
        }
        break;

    case WarningState::Shrinking:
        band_half_height -= band_expand_speed * delta_second;
        if (band_half_height <= 0.0f)
        {
            band_half_height = 0.0f;
            warning_state = WarningState::None;
        }
        // �X�e�[�W4��BGM�Đ�
        PlaySoundMem(stage_bgm4, DX_PLAYTYPE_LOOP);
        ChangeVolumeSoundMem(255 * 100 / 100, stage_bgm4);

        if (player)
            player->SetShotStop(false);

        break;

    default:
        break;
    }
}

#define D 0
void GameMainScene::DrawGameOverEffect() {

    int gameover_alpha = 0;

    if (gameover_timer < 0.5f)
    {
        // 0�`130�̊Ԃ�sin�g�ō����ɉ����i���g�������߂�j
        float wave = sinf(gameover_timer * 80.0f); // �������Z���i�����j
        gameover_alpha = static_cast<int>((wave + 1.0f) * 0.5f * 130.0f); // �� 0�`130

#if D
        // ���[�ɐԂ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, 290, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        DrawBox(990, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
#else
        // �Ԃ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

#endif

    }
    else if (gameover_timer < 3.5f)
    {
        gameover_alpha = 90;

#if D
        // ���[�ɐԂ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, 290, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        DrawBox(990, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
#else
        // �Ԃ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);


        // �����m�C�Y��`��i�Ԃ��т������_���ɕ\���j
        for (int i = 0; i < 200; ++i)
        {
            int x = 0;
            int y = rand() % D_WIN_MAX_Y;
            int w = D_WIN_MAX_X;
            int h = 2 + rand() % 4; // ����2?5�s�N�Z�����x
            int r = 220 + rand() % 35;
            int g = rand() % 40;
            int b = rand() % 40;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
            DrawBox(x, y, x + w, y + h, GetColor(r, g, b), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

#endif

    }
    else
    {
        // 2�b�o�ߌ�͌Œ�
        gameover_alpha = 110;

#if D
        // ���[�ɐԂ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, 290, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        DrawBox(990, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
#else
        // �Ԃ��t�B���^�[��`��
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, gameover_alpha);
        DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

#endif

        SetFontSize(54);
        DrawString(D_WIN_MAX_X / 2 - 200, D_WIN_MAX_Y / 2 - 32, "GAME OVER", GetColor(255, 255, 255));
        SetFontSize(16);
    }
}


eSceneType GameMainScene::UpdateGameplay(float delta)
{
    // �^�C�}�[�X�V�i���t�F�[�h��UI�p�j
    black_in_timer += delta;
    line_effect_timer += delta;
    eSceneType scene_type = GetNowSceneType();

    // �G�t�F�N�g�X�V
    if (auto* effectMgr = Singleton<EffectManager>::GetInstance()) {
        effectMgr->Update(delta);
    }

    // �X�e�[�W�X�V
    if (current_stage) {
        current_stage->Update(delta);
    }

    // ��ʃV�F�C�N�X�V
    if (auto* shakeMgr = Singleton<ShakeManager>::GetInstance()) {
        if (player && player->GetIsAlive()) {
            shakeMgr->Update(delta);
        }
    }

    // === �X�e�[�W4 �x�����o ===
    if (current_stage && current_stage->GetStageID() == StageID::Stage4) {
        red_alpha_timer += delta * DX_PI;  // �_�ő��x
        WarningUpdate(delta);              // �x���A�j���i�s
    }

    InputManager* input = Singleton<InputManager>::GetInstance();

    // === �X�e�[�W�N���A�E�J�ڏ��� ===
    if (current_stage && current_stage->IsFinished() || input->GetKeyDown(KEY_INPUT_1)) {
        if (current_stage->IsClear() || input->GetKeyDown(KEY_INPUT_1)) {
            scene_type = ProceedToNextStage(delta);
        }
        else if (current_stage->IsOver()) {
            scene_type = UpdateGameOverState(delta);
        }
    }

    return scene_type;
}

eSceneType GameMainScene::ProceedToNextStage(float delta)
{
    StageID id = current_stage->GetStageID();

    // �X�e�[�W3 �� �X�e�[�W4 ��p�t�F�[�h���o
    if (id == StageID::Stage3)
    {
        black_fade_timer += delta;
        if (alpha >= 255)
        {
            StopSoundMem(stage_bgm3);
            StageBase* next_stage = current_stage->GetNextStage(player);

            current_stage->Finalize();
            delete current_stage;
            current_stage = nullptr;

            if (next_stage)
            {
                current_stage = next_stage;
                current_stage->Initialize();
                current_bgm_handle = stage_bgm4;
            }
            else
            {
                return eSceneType::eTitle;
            }
        }
    }
    else if (id == StageID::Stage4)
    {
        end_sequence_started = true;
        if (player) player->SetShotStop(true);

        end_sequence_timer += delta;
        const float fade_duration = 2.0f;
        if (end_sequence_timer >= 1.0f && end_sequence_timer < 1.0f + fade_duration)
        {
            float t = (end_sequence_timer - 1.0f) / fade_duration;
            end_fade_alpha = static_cast<int>(255 * Min(t, 1.0f));
        }
        else if (end_sequence_timer >= 8.0f)
        {
            current_stage->Finalize();
            delete current_stage;
            current_stage = nullptr;
            return eSceneType::eTitle;
        }
    }
    else
    {
        StageBase* next_stage = current_stage->GetNextStage(player);

        current_stage->Finalize();
        delete current_stage;
        current_stage = nullptr;

        if (next_stage)
        {
            current_stage = next_stage;
            current_stage->Initialize();

            if (current_stage->GetStageID() == StageID::Stage3)
            {
                StopSoundMem(current_bgm_handle);
                current_bgm_handle = stage_bgm3;
                ChangeVolumeSoundMem(255 * 95 / 100, current_bgm_handle);
                PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);
            }
        }
        else
        {
            return eSceneType::eTitle;
        }
    }

    return GetNowSceneType();
}

eSceneType GameMainScene::UpdateGameOverState(float delta)
{
    gameover_timer += delta;

    if (gameover_timer > 5.0f)
    {

        retry = true;
        InputManager* input = Singleton<InputManager>::GetInstance();

        if (input->GetKeyDown(KEY_INPUT_UP) || input->GetKeyDown(KEY_INPUT_W) ||
            input->GetButtonDown(XINPUT_BUTTON_DPAD_UP)) {
            PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
            m_selectedIndex = (m_selectedIndex + 1) % 2;
        }

        if (input->GetKeyDown(KEY_INPUT_DOWN) || input->GetKeyDown(KEY_INPUT_S) ||
            input->GetButtonDown(XINPUT_BUTTON_DPAD_DOWN)) {
            PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
            m_selectedIndex = (m_selectedIndex + 1) % 2;
        }

        if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A)) {
            if (m_selectedIndex == 0) {
                // === RETRY ===
                GameObjectManager::GetInstance()->Finalize();
                ScoreData::GetInstance()->Reset();

                if (current_stage) {
                    StageID id = current_stage->GetStageID();
                    current_stage->Finalize();
                    delete current_stage;
                    current_stage = nullptr;

                    player = GameObjectManager::GetInstance()->CreateObject<Player>(
                        Vector2D(D_WIN_MAX_X / 2, (D_WIN_MAX_Y / 2) + 220.0f)
                    );

                    switch (id) {
                    case StageID::Stage1: current_stage = new Stage1(player); break;
                    case StageID::Stage2: current_stage = new Stage2(player); break;
                    case StageID::Stage3: current_stage = new Stage3(player); break;
                    case StageID::Stage4: current_stage = new Stage4(player); break;
                    }

                    if (current_stage) current_stage->Initialize();
                    gameover_timer = 0.0f;
                    retry = false;
                    m_selectedIndex = 0;

                    if (current_stage && current_stage->GetStageID() == StageID::Stage3)
                    {
                        StopSoundMem(current_bgm_handle);
                        current_bgm_handle = stage_bgm3;
                        ChangeVolumeSoundMem(255 * 90 / 100, current_bgm_handle);
                        PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);
                    }

                    if (current_stage && current_stage->GetStageID() == StageID::Stage4)
                    {
                        StopSoundMem(current_bgm_handle);
                        current_bgm_handle = stage_bgm4;
                        ChangeVolumeSoundMem(255 * 100 / 100, current_bgm_handle);
                        PlaySoundMem(current_bgm_handle, DX_PLAYTYPE_LOOP);
                    }

                    return GetNowSceneType();
                }
            }
            else if (m_selectedIndex == 1) {
                // === BACK TITLE ===
                GameObjectManager::GetInstance()->Finalize();
                return eSceneType::eTitle;
            }
        }
    }

    return GetNowSceneType();
}

eSceneType GameMainScene::UpdatePauseMenu(float delta)
{
    InputManager* input = Singleton<InputManager>::GetInstance();

    // ==========================
    // �����x�t�F�[�h�i�Ó]�j
    // ==========================
    pause_timer += delta;
    if (pause_timer >= 0.01f)
    {
        transparent++;
        pause_timer = 0.0f;
    }
    if (transparent >= 100)
        transparent = 100;

    // ==========================
    // �J�[�\���ړ��i�㉺�j
    // ==========================
    bool movedUp = input->GetKeyDown(KEY_INPUT_UP) ||
        input->GetKeyDown(KEY_INPUT_W) ||
        input->GetButtonDown(XINPUT_BUTTON_DPAD_UP);

    bool movedDown = input->GetKeyDown(KEY_INPUT_DOWN) ||
        input->GetKeyDown(KEY_INPUT_S) ||
        input->GetButtonDown(XINPUT_BUTTON_DPAD_DOWN);

    // �X�e�B�b�N���́i�㉺�j + �N�[���_�E��
    static float stickCooldown = 0.0f;
    stickCooldown -= delta;

    Vector2D stick = input->GetLeftStick();
    const float STICK_THRESHOLD = 0.5f;
    const float COOLDOWN_TIME = 0.2f;

    if (stickCooldown <= 0.0f)
    {
        if (stick.y > STICK_THRESHOLD)
        {
            movedUp = true;
            stickCooldown = COOLDOWN_TIME;
        }
        else if (stick.y < -STICK_THRESHOLD)
        {
            movedDown = true;
            stickCooldown = COOLDOWN_TIME;
        }
    }

    if (movedUp)
    {
        PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
        m_selectedIndex = (m_selectedIndex + 1) % 2;
    }

    if (movedDown)
    {
        PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
        m_selectedIndex = (m_selectedIndex + 1) % 2;
    }

    // ==========================
    // ���菈���i�X�y�[�X or A�{�^���j
    // ==========================
    if (!m_startTransitioning &&
        (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A)))
    {
        if (m_selectedIndex == 0)
        {
            // === �Q�[���ĊJ ===
            isPaused = false;
        }
        else if (m_selectedIndex == 1)
        {
            // === �^�C�g���֖߂� ===
            auto* objm = Singleton<GameObjectManager>::GetInstance();
            if (objm) objm->Finalize();

            return eSceneType::eTitle;
        }
    }
    return GetNowSceneType();
}

void GameMainScene::StartUITransition(PlayerType new_type)
{
    // ���łɐ؂�ւ����Ȃ疳������
    if (ui_transitioning) return;

    // �����^�C�v�Ȃ疳��
    if (new_type != ui_current_type)
    {
        ui_target_type = new_type;
        ui_transition_timer = 0.0f;
        ui_transitioning = true;
    }
}

void GameMainScene::UpdateUITransition(float delta)
{
#if UI_CHANGE
    player->SetCanChangeType(!ui_transitioning);

    if (!ui_transitioning) return;

    ui_transition_timer += delta;

    // �m�C�Y�h��̉��o�i���Ԃōł��傫���j
    float t = ui_transition_timer / UI_TRANSITION_DURATION;
    float noise_strength = (1.0f - fabs(1.0f - t * 2)) * 10.0f;
    Singleton<ShakeManager>::GetInstance()->StartShake(0.05f, noise_strength, noise_strength);

    if (ui_transition_timer >= UI_TRANSITION_DURATION)
    {
        Singleton<ShakeManager>::GetInstance()->StartShake(0.5f, 10, 0); // ���莞�̉��o
        ui_transitioning = false;
        ui_current_type = ui_target_type;
        ui_transition_timer = 0.0f;
    }

#else

    // UI���o���͐؂�ւ��s�ɂ���
    player->SetCanChangeType(!ui_transitioning);

    if (!ui_transitioning) return;

    ui_transition_timer += delta;
    if (ui_transition_timer >= UI_TRANSITION_DURATION)
    {
        Singleton<ShakeManager>::GetInstance()->StartShake(0.5, 10, 0);
        ui_transitioning = false;
        ui_current_type = ui_target_type; // ��Ԃ��m��
        ui_transition_timer = 0.0f;       // �O�̂���
    }
#endif

}