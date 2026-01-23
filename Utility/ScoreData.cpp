#include "ScoreData.h"
#include "DxLib.h"
#include <ctime>
#include <algorithm>
#include <fstream>

ScoreData::ScoreData() : stage_time(4, { 0.0f, 0.0f, 0.0f })
{
}

ScoreData::~ScoreData()
{
}

std::vector<float> ScoreData::GetScoreData() const
{
	return score_data;
}

void ScoreData::Reset()
{
	submitted = false;
	score_data.clear();
	player_stock = 0;
}

void ScoreData::ScoreEntrie(float total_score)
{
    // šˆê“x‚¾‚¯“o˜^
    if (total_score > 0.9f)
    {
        entries.push_back({ total_score, GetTodayString() });
        std::sort(entries.begin(), entries.end(), [](auto& a, auto& b) {
            return a.score > b.score;
            });
        if (entries.size() > 10)
            entries.resize(10);
        SaveRankingToFile();
    }
}

void ScoreData::AddScore(float value)
{
	score_data.push_back(value);
}

float ScoreData::GetTotalScore() const
{
	float total = 0.0f;
	for (float s : score_data)
		total += s;
	return total;
}

std::string ScoreData::GetTodayString()
{
    time_t now = time(nullptr);
    tm t;
    localtime_s(&t, &now);
    char buf[32];
    sprintf_s(buf, "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    return std::string(buf);
}

void ScoreData::SaveRankingToFile()
{
    std::ofstream file("ranking.txt", std::ios::trunc);
    for (const auto& e : entries)
        file << e.score << "," << e.date << "\n";
    file.close();
}

void ScoreData::SetResetRanking()
{
    entries.clear();
}

std::vector<ScoreEntry> ScoreData::GetEntries() const
{
    return entries;
}

void ScoreData::SetStageScore(int stage_num, float stage_score)
{
    if (stage_num <= 1)
    {
        if(stage1_score <= stage_score)
            stage1_score = stage_score;
    }
    else if (stage_num == 2)
    {
        if (stage2_score <= stage_score)
            stage2_score = stage_score;
    }
    else
    {
        if (stage34_score <= stage_score)
            stage34_score = stage_score;
    }
}

float ScoreData::GetStageScore(int stage_num)
{
    if (stage_num <= 1)
        return stage1_score;
    else if (stage_num == 2)
        return stage2_score;
    else
        return stage34_score;
}

void ScoreData::SetTime(int stage_num, float hun, float byou, float miri)
{
    if (stage_time[stage_num].time_hun <= hun)
    {
        stage_time[stage_num].time_hun = hun;
        stage_time[stage_num].time_byou = byou;
        stage_time[stage_num].time_miri = miri;
    }
    else if (stage_time[stage_num].time_byou <= byou)
    {
        stage_time[stage_num].time_byou = byou;
        stage_time[stage_num].time_miri = miri;
    }
    else if (stage_time[stage_num].time_miri <= miri)
    {
        stage_time[stage_num].time_miri = miri;
    }
}

Time ScoreData::GetTime(int stage_num)
{
    if (stage_num <= 1)
        return stage_time[stage_num];
    else if (stage_num == 2)
        return stage_time[stage_num];
    else
        return stage_time[stage_num];
}


