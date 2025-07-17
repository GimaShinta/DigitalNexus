#include "RankingScene.h"
#include "../../Utility/ScoreData.h"
#include <fstream>

RankingScene::RankingScene()
{
}

RankingScene::~RankingScene()
{
}

// 初期化処理
void RankingScene::Initialize()
{
    total_score = 0.0f;
    start_time = GetNowCount();
    confirm_reset = false;
    cursor_index = 0;

    auto* score_data = Singleton<ScoreData>::GetInstance();

    // スコア合計
    total_score = score_data->GetTotalScore();

    LoadRankingFromFile();

    //// ★一度だけ登録
    //if (total_score > 0.9f && !score_data->IsSubmitted())
    //{
    //    entries.push_back({ total_score, GetTodayString() });
    //    std::sort(entries.begin(), entries.end(), [](auto& a, auto& b) {
    //        return a.score > b.score;
    //        });
    //    if (entries.size() > 10)
    //        entries.resize(10);
    //    SaveRankingToFile();

    //    // ★登録済みにマーク
    //    score_data->MarkSubmitted();
    //}

    entries = ScoreData::GetInstance()->GetEntries();
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType RankingScene::Update(float delta_second)
{
	//InputManager* input = Singleton<InputManager>::GetInstance();

	//if (input->GetKeyDown(KEY_INPUT_SPACE))
	//	return eSceneType::eCredit;

    InputManager* input = Singleton<InputManager>::GetInstance();

    if (confirm_reset)
    {
        if (input->GetKeyDown(KEY_INPUT_X) || input->GetButtonDown(XINPUT_BUTTON_A))
        {
            // ★ すべてのランキングを削除
            entries.clear();
            SaveRankingToFile();
            confirm_reset = false;
            cursor_index = 0;
        }
        if (input->GetButtonDown(XINPUT_BUTTON_B))
        {
            confirm_reset = false;
        }
    }
    else
    {
        if (!entries.empty())
        {
            if (input->GetKeyDown(KEY_INPUT_DOWN) || input->GetButtonDown(XINPUT_BUTTON_DPAD_DOWN))
                cursor_index = (cursor_index + 1) % entries.size();

            if (input->GetKeyDown(KEY_INPUT_UP) || input->GetButtonDown(XINPUT_BUTTON_DPAD_UP))
                cursor_index = (cursor_index + entries.size() - 1) % entries.size();
        }

        if (input->GetKeyDown(KEY_INPUT_Z) || input->GetKeyDown(KEY_INPUT_RETURN) || input->GetButtonDown(XINPUT_BUTTON_B))
            return eSceneType::eTitle;

        if (input->GetKeyDown(KEY_INPUT_X) || input->GetButtonDown(XINPUT_BUTTON_X))
        {
            //confirm_reset = true;
        }

        if (input->GetButton(XINPUT_BUTTON_LEFT_SHOULDER))
            reset_command[0] = true;
        if (input->GetButton(XINPUT_BUTTON_RIGHT_SHOULDER))
            reset_command[1] = true;
        if (input->GetButton(XINPUT_BUTTON_DPAD_RIGHT))
            reset_command[2] = true;
        if (input->GetButton(XINPUT_BUTTON_X))
            reset_command[3] = true;

        if (reset_command[0] && reset_command[1] && reset_command[2] && reset_command[3])
        {
            // ★ すべてのランキングを削除
            entries.clear();
            SaveRankingToFile();
            confirm_reset = false;
            cursor_index = 0;
        }

    }

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void RankingScene::Draw()
{
#if _DEBUG
	DrawString(0, 0, "RankingScene", GetColor(255, 255, 255));
#endif

    DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(10, 10, 30), TRUE);

    for (int y = 0; y < D_WIN_MAX_Y; y += 4)
    {
        int bright = 10 + (y + GetNowCount() / 3) % 40;
        DrawLine(0, y, D_WIN_MAX_X, y, GetColor(0, bright, bright));
    }

    int cx = D_WIN_MAX_X / 2;
    int elapsed = GetNowCount() - start_time;
    float scale = (elapsed < 60) ? 1.5f - (elapsed / 60.0f) * 0.5f : 1.0f;

    SetFontSize(32);
    if (entries.empty())
    {
        DrawNeonText(cx - 100, 300, "NO DATA", GetColor(100, 100, 100), GetColor(0, 100, 255), 120);
        return;
    }
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawExtendString(cx - 180 * scale, 50, scale, scale, "HIGH SCORE RANKING", GetColor(0, 255, 255));
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // 修正済み描画位置
    int panel_w = 680;
    int bx = cx - panel_w / 2;
    int rank_x = bx + 30;
    int score_x = bx + 110;
    int date_x = bx + 340;

    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
        const auto& e = entries[i];
        int y = 140 + i * 48;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(bx, y - 4, bx + panel_w, y + 40, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
        DrawBox(bx, y - 4, bx + panel_w, y + 40, GetColor(0, 255, 255), FALSE);

        int textColor = (i == 0) ? GetColor(255, 215, 0) : GetColor(255, 255, 255);
        if (i == 0 && GetNowCount() % 100 > 50)
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        DrawFormatString(rank_x, y, textColor, "%2d:", i + 1);
        DrawFormatString(score_x, y, textColor, "%6.0f", e.score);
        DrawString(date_x, y, e.date.c_str(), textColor);

        if (i == 0)
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    // カーソルと再描画枠
    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
        const auto& e = entries[i];
        int y = 140 + i * 48;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(bx, y - 4, bx + panel_w, y + 40, GetColor(0, 0, 0), TRUE);

        if (i == cursor_index)
        {
            DrawBox(bx - 4, y - 8, bx + panel_w + 4, y + 44, GetColor(255, 255, 0), FALSE);
            DrawString(bx - 30, y, "", GetColor(255, 255, 0));
        }
        else
        {
            DrawBox(bx, y - 4, bx + panel_w, y + 40, GetColor(0, 255, 255), FALSE);
        }

        int textColor = (i == 0) ? GetColor(255, 215, 0) : GetColor(255, 255, 255);
        if (i == 0 && GetNowCount() % 100 > 50)
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

        DrawFormatString(rank_x, y, textColor, "%2d:", i + 1);
        DrawFormatString(score_x, y, textColor, "%6.0f", e.score);
        DrawString(date_x, y, e.date.c_str(), textColor);

        if (i == 0)
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    const int msg_y = 640;
    if (confirm_reset)
    {
        DrawNeonText(cx - 260, msg_y,
            "Reset ranking?   A = Yes     B = No",
            GetColor(255, 120, 120), GetColor(255, 0, 0), 180);
    }
    else
    {
        DrawNeonText(cx - 260, msg_y,
            "B : Title",
            GetColor(100, 255, 255), GetColor(0, 150, 255), 150);
    }

    SetFontSize(16);
}

// 終了時処理（使ったインスタンスの削除とか）
void RankingScene::Finalize()
{
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType RankingScene::GetNowSceneType() const
{
	return eSceneType::eRanking;
}

bool RankingScene::LoadRankingFromFile()
{
    entries.clear();
    std::ifstream file("ranking.txt");

    if (!file.is_open())
    {
        // ファイルがなければ空のまま返す（ダミーデータを入れない）
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        ScoreEntry entry;
        size_t p = line.find(",");
        if (p == std::string::npos) continue;

        entry.score = std::stof(line.substr(0, p));
        entry.date = line.substr(p + 1);
        entries.push_back(entry);
    }

    file.close();
    return true;
}

void RankingScene::SaveRankingToFile()
{
    std::ofstream file("ranking.txt", std::ios::trunc);
    for (const auto& e : entries)
        file << e.score << "," << e.date << "\n";
    file.close();
}

void RankingScene::ResetRankingFile()
{
    std::ofstream file("ranking.txt", std::ios::trunc);  // 空ファイル書き込み
    file.close();
    LoadRankingFromFile();
}

//std::string RankingScene::GetTodayString()
//{
//    time_t now = time(nullptr);
//    tm t;
//    localtime_s(&t, &now);
//    char buf[32];
//    sprintf_s(buf, "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
//    return std::string(buf);
//}

void RankingScene::DrawNeonText(int x, int y, const char* text, int baseColor, int glowColor, int glowPulse)
{
    int pulse = (GetNowCount() % 100 > 50) ? glowPulse : 255;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse);
    DrawString(x + 2, y + 2, text, glowColor);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    DrawString(x, y, text, baseColor);
}