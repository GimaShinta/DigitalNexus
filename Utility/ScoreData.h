#pragma once
#include "../Singleton.h"
#include <vector>
#include <string>

struct ScoreEntry
{
	float score;
	std::string date;
};

class ScoreData :
    public Singleton<ScoreData>
{
	//スコア
	std::vector<float> score_data;
	int player_stock;	//残機
	bool submitted = false;
	std::vector<ScoreEntry> entries;

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
	std::vector<ScoreEntry> GetEntries() const;
};

