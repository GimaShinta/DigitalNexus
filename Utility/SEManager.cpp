#include "SEManager.h"
#include "ResourceManager.h"
#include "DxLib.h"

void SEManager::LoadSE()
{
	WarmUpSE();
	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();

	se_handles[SE_NAME::Shot] = rm->GetSounds("Resource/sound/se/shot/shot_02.mp3");
	se_handles[SE_NAME::Destroy] = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
	se_handles[SE_NAME::Hit] = rm->GetSounds("Resource/sound/se/se_effect/hit_se.mp3");
	//se_handles[SE_NAME::Reaction] = rm->GetSounds("Resource/sound/se/se_effect/reaction.mp3");
	//se_handles[SE_NAME::Explosion] = rm->GetSounds("Resource/sound/se/se_effect/explosion.mp3"); // 仮に追加

	se_handles[SE_NAME::Bakuhatu] = rm->GetSounds("Resource/sound/se/battle/bakuhatu_b.mp3");
	se_handles[SE_NAME::Kill] = rm->GetSounds("Resource/sound/se/boss_se/boss_kill.mp3");
	se_handles[SE_NAME::Bakuhatu_End] = rm->GetSounds("Resource/sound/se/boss_se/bakuhatu_end.mp3");
	se_handles[SE_NAME::Hamon] = rm->GetSounds("Resource/sound/se/boss_se/hamon.mp3");
	se_handles[SE_NAME::EnemyShot] = rm->GetSounds("Resource/sound/se/shot/shot_01.mp3");
	se_handles[SE_NAME::EnemyBeam] = rm->GetSounds("Resource/sound/se/effect/audiostock_1553653.mp3");
	//ChangeVolumeSoundMem(255 * 100 / 100, se_handles[SE_NAME::Bakuhatu]);
	//ChangeVolumeSoundMem(255 * 100 / 100, se_handles[SE_NAME::Kill]);
	//ChangeVolumeSoundMem(255 * 100 / 100, se_handles[SE_NAME::Bakuhatu_End]);
	se_handles[SE_NAME::Get] = rm->GetSounds("Resource/sound/se/effect/audiostock_890909.mp3");
	se_handles[SE_NAME::Get2] = rm->GetSounds("Resource/sound/se/effect/audiostock_1133382.mp3");
	se_handles[SE_NAME::PlayerBeam] = rm->GetSounds("Resource/sound/se/effect/audiostock_1244545.mp3");
}

void SEManager::UnloadSE()
{
	for (auto& pair : se_handles)
	{
		if (pair.second != -1) // -1 は無効なハンドル
		{
			DeleteSoundMem(pair.second);
			pair.second = -1; // 再利用・再解放防止のため無効化
		}
	}
}

void SEManager::WarmUpSE()
{
	for (auto it = se_handles.begin(); it != se_handles.end(); ++it)
	{
		SE_NAME name = it->first;
		int handle = it->second;

		PlaySoundMem(handle, DX_PLAYTYPE_BACK);
		StopSoundMem(handle);
	}

	//#ifndef NDEBUG
	//	printfDx("SEウォームアップ完了（%d種）\n", (int)se_handles.size());
	//#endif

}

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

void SEManager::ChangeSEVolume(SE_NAME se, int volume)
{
	// volume: 0?100 の範囲を想定（DxLib内部は 0?255）

	if (se_handles.count(se) == 0) return; // 存在確認
	int handle = se_handles[se];

	if (handle != -1)
	{
		int dx_volume = Clamp(volume * 255 / 100, 0, 255); // 変換＋制限
		ChangeVolumeSoundMem(dx_volume, handle);
	}
}
