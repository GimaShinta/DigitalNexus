#pragma once
#include "../Singleton.h"
#include <map>

enum class SE_NAME
{
	Shot,
	Reaction,
	Explosion,
	Bakuhatu,
	Bakuhatu_End,
	Kill,
	Destroy,
	Hit,
	EnemyShot,
	Hamon,
	EnemyBeam
	// �K�v�ɉ����Ēǉ�
};

class SEManager : public Singleton<SEManager>
{
public:
	SEManager() = default;

public:
	void LoadSE();
	void WarmUpSE();
	void PlaySE(SE_NAME name);
	void UnloadSE();
	void ChangeSEVolume(SE_NAME se, int volume);

private:
	std::map<SE_NAME, int> se_handles; // SE�̃n���h���}�b�v

	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}
};

