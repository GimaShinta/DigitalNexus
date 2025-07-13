#include "ScoreData.h"
#include "DxLib.h"
#include <ctime>
#include <algorithm>
#include <fstream>

ScoreData::ScoreData()
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

std::vector<ScoreEntry> ScoreData::GetEntries() const
{
    return entries;
}
