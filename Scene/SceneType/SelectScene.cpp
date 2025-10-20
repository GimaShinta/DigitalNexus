#include "SelectScene.h"

SelectScene::SelectScene()
{
}

SelectScene::~SelectScene()
{
}

// 初期化処理
void SelectScene::Initialize()
{
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <returns></returns>
eSceneType SelectScene::Update(float delta_second)
{
	// 入力情報の取得
	InputManager* input = Singleton<InputManager>::GetInstance();

	// カーソルの移動範囲
	int cur_max;
	int cur_min;

	// ステージ選択のカーソル
	if (stage_check == false)
	{
		int cur_max = 3;
		int cur_min = 1;

		if (input->GetKeyDown(KEY_INPUT_D) || input->GetButtonDown(XINPUT_BUTTON_DPAD_RIGHT))
		{
			if (stage_cursor < cur_max)
			{
				stage_cursor++;
			}
		}
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_DPAD_LEFT))
		{
			if (stage_cursor > cur_min)
			{
				stage_cursor--;
			}
		}

		// ステージの選択をする
		if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A))
			stage_check = true;
	}
	// ステージ選択の確認の操作
	else
	{
		int cur_max = 2;
		int cur_min = 1;

		if (input->GetKeyDown(KEY_INPUT_D) || input->GetButtonDown(XINPUT_BUTTON_DPAD_RIGHT))
		{
			if (check_cursor < cur_max)
			{
				check_cursor++;
			}
		}
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_DPAD_LEFT))
		{
			if (check_cursor > cur_min)
			{
				check_cursor--;
			}
		}

		// カーソルが指す動作を開始
		if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A))
		{
			// 戻る
			if (check_cursor == 1)
			{
				stage_check = false;
				check_cursor = cur_min;
			}
			// 決定
			else
			{
				return eSceneType::eGameMain;
			}
		}

		// カーソル以外での戻るボタン
		if (input->GetKeyDown(KEY_INPUT_Z) || input->GetButtonDown(XINPUT_BUTTON_B))
		{
			stage_check = false;
			check_cursor = cur_min;
		}
	}

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void SelectScene::Draw()
{
	DrawString(0, 0, "selectscene", GetColor(255, 255, 255), TRUE);
	DrawFormatString(0, 30, GetColor(255, 255, 255), "selectstage : %d", stage_cursor);
	DrawFormatString(0, 60, GetColor(255, 255, 255), "selectstage : %d", check_cursor);

	// オフセット
	float start_x = 100.0f;
	float start_y = 150.0f;
	float box_offset = 250.0f;
	int select_color = GetColor(255, 255, 255);
	// ステージ詳細のボックス
	for (int i = 0; i < 3; i++)
	{
		DrawBox(start_x + (i * 400), start_y, start_x + box_offset + (i * 400), start_y + box_offset,
			select_color, TRUE);
		DrawFormatString(start_x + (i * 400), start_y, GetColor(255, 0, 0), "Stage %d", i + 1);
	}

	// ステージ選択のカーソル
	if (stage_check == false)
	{
		// カーソルの表示
		DrawBox(start_x + ((stage_cursor * 400) - 400), start_y, start_x + box_offset + ((stage_cursor * 400) - 400), start_y + box_offset,
			GetColor(0, 255, 0), TRUE);
	}
	// ステージ選択の確認のカーソル
	else
	{
		// 確認ボックス
		for (int i = 0; i < 2; i++)
		{
			DrawBox(800 + (i * 200), 650, 150 + 800 + (i * 200), 700,
				select_color, TRUE);
		}

		// カーソルの表示
		DrawBox(800 + ((check_cursor * 200) - 200), 650, 150 + 800 + ((check_cursor * 200) - 200), 700,
			GetColor(0, 255, 0), TRUE);
		DrawString(800, 650, "Back", GetColor(0, 0, 255), TRUE);
		DrawString(1000, 650, "ok", GetColor(0, 0, 255), TRUE);

	}
}

// 終了時処理（使ったインスタンスの削除とか）
void SelectScene::Finalize()
{
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType SelectScene::GetNowSceneType() const
{
	return eSceneType::eSelect;
}
