#pragma once
#include "../Singleton.h"
#include <vector>
#include <string>

struct ScoreEntry
{
	float score;
	std::string date;
};

struct Time
{
	float time_hun;
	float time_byou;
	float time_miri;
};

class ScoreData :
    public Singleton<ScoreData>
{
	//スコア
	std::vector<float> score_data;
	int player_stock;	//残機
	bool submitted = false;
	std::vector<ScoreEntry> entries;
	float stage1_score = 0;
	float stage2_score = 0;
	float stage34_score = 0;

	std::vector<Time> stage_time;

public:
	ScoreData();
	~ScoreData();

public:

	std::vector<float> GetScoreData() const;
	void AddScore(float value);
	float GetTotalScore() const;
	void Reset();
	bool IsSubmitted() const { return submitted; }
	void MarkSubmitted() { submitted = true; }
	void ScoreEntrie(float total_score);
	std::string GetTodayString();
	void SaveRankingToFile();
	void SetResetRanking();
	std::vector<ScoreEntry> GetEntries() const;
	void SetStageScore(int stage_num, float stage_score);
	float GetStageScore(int stage_num);
	void SetTime(int stage_num, float hun, float byou, float miri);
	Time GetTime(int stage_num);
};

