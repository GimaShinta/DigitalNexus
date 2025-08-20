#include "SEManager.h"
#include "ResourceManager.h"
#include "DxLib.h"

// SEの読み込み（事前読み込み）
void SEManager::LoadSE()
{
	WarmUpSE();
	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();

	se_handles[SE_NAME::Shot] = rm->GetSounds("Resource/sound/se/shot/shot_02.mp3");
	se_handles[SE_NAME::Destroy] = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
	se_handles[SE_NAME::Hit] = rm->GetSounds("Resource/sound/se/se_effect/hit_se.mp3");
	se_handles[SE_NAME::Bakuhatu] = rm->GetSounds("Resource/sound/se/battle/bakuhatu_b.mp3");
	se_handles[SE_NAME::Kill] = rm->GetSounds("Resource/sound/se/boss_se/boss_kill.mp3");
	se_handles[SE_NAME::Bakuhatu_End] = rm->GetSounds("Resource/sound/se/boss_se/bakuhatu_end.mp3");
	se_handles[SE_NAME::Hamon] = rm->GetSounds("Resource/sound/se/boss_se/hamon.mp3");
	se_handles[SE_NAME::EnemyShot] = rm->GetSounds("Resource/sound/se/shot/shot_01.mp3");
	se_handles[SE_NAME::EnemyBeam] = rm->GetSounds("Resource/sound/se/effect/audiostock_1553653.mp3");
	se_handles[SE_NAME::Get] = rm->GetSounds("Resource/sound/se/effect/audiostock_890909.mp3");
	se_handles[SE_NAME::Get2] = rm->GetSounds("Resource/sound/se/effect/audiostock_1133382.mp3");
	se_handles[SE_NAME::PlayerBeam] = rm->GetSounds("Resource/sound/se/effect/audiostock_1244545.mp3");
	se_handles[SE_NAME::Robo1] = rm->GetSounds("Resource/sound/se/effect/robo_se.mp3");
	se_handles[SE_NAME::Robo2] = rm->GetSounds("Resource/sound/se/effect/robo2_se.mp3");
	se_handles[SE_NAME::Robo3] = rm->GetSounds("Resource/sound/se/effect/robo3_se.mp3");
	se_handles[SE_NAME::Code] = rm->GetSounds("Resource/sound/se/effect/code_se.mp3");
	se_handles[SE_NAME::Code2] = rm->GetSounds("Resource/sound/se/effect/code2_se.mp3");
	se_handles[SE_NAME::Noise] = rm->GetSounds("Resource/sound/se/effect/noise_se.mp3");
	se_handles[SE_NAME::Dead1] = rm->GetSounds("Resource/sound/se/effect/dead_se.mp3");
}

// SEの削除
void SEManager::UnloadSE()
{
	for (auto& pair : se_handles)
	{
		if (pair.second != -1) 
		{
			DeleteSoundMem(pair.second);
			pair.second = -1;
		}
	}
}

// 予め再生させておく
void SEManager::WarmUpSE()
{
	for (auto it = se_handles.begin(); it != se_handles.end(); ++it)
	{
		SE_NAME name = it->first;
		int handle = it->second;

		PlaySoundMem(handle, DX_PLAYTYPE_BACK);
		StopSoundMem(handle);
	}
}

// 指定したSEを再生する
void SEManager::PlaySE(SE_NAME name)
{
	if (se_handles.count(name))
	{
		PlaySoundMem(se_handles[name], DX_PLAYTYPE_BACK);
	}
#ifndef NDEBUG
	else
	{
		printfDx("未登録のSEが呼ばれました: %d\n", static_cast<int>(name));
	}
#endif
}

// 指定したSEのボリュームを設定する
void SEManager::ChangeSEVolume(SE_NAME se, int volume)
{

	if (se_handles.count(se) == 0) return; // 存在確認
	int handle = se_handles[se];

	if (handle != -1)
	{
		int dx_volume = Clamp(volume * 255 / 100, 0, 255); 
		ChangeVolumeSoundMem(dx_volume, handle);
	}
}
