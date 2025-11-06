#include "SelectScene.h"
#include "../../Utility/SelectStage.h"

SelectScene::SelectScene()
{
}

SelectScene::~SelectScene()
{
}

// 初期化処理
void SelectScene::Initialize()
{
	// 画像読み込み機能取得
	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	stage_image[0] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage1.png")[0];
	stage_image[1] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage2.png")[0];
	stage_image[2] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage3.png")[0];
	stage_image[3] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage1_2.png")[0];
	stage_image[4] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage2_2.png")[0];
	stage_image[5] = rm->GetImages("Resource/Image/BackGround/Select/select_image/stage3_2.png")[0];

	// 動画読み込み
	stage_movie[0] = LoadGraph("Resource/pv/stage_pv/Stage1_movie.mp4");
	stage_movie[1] = LoadGraph("Resource/pv/stage_pv/Stage2_movie.mp4");
	stage_movie[2] = LoadGraph("Resource/pv/stage_pv/Stage3_movie.mp4");

	// 動画再生
	PlayMovieToGraph(stage_movie[0], DX_PLAYTYPE_LOOP);
	PlayMovieToGraph(stage_movie[1], DX_PLAYTYPE_LOOP);
	PlayMovieToGraph(stage_movie[2], DX_PLAYTYPE_LOOP);

	PauseMovieToGraph(stage_movie[0]);
	PauseMovieToGraph(stage_movie[1]);
	PauseMovieToGraph(stage_movie[2]);
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
		cur_max = 3;
		cur_min = 1;

		// カーソルを右に移動
		if (input->GetKeyDown(KEY_INPUT_D) || input->GetButtonDown(XINPUT_BUTTON_DPAD_RIGHT))
		{
			if (stage_cursor < cur_max)
			{
				stage_cursor++;
			}
		}
		// カーソルを左に移動
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

		// タイトル画面へ戻る
		if (input->GetKeyDown(KEY_INPUT_Z) || input->GetButtonDown(XINPUT_BUTTON_B))
			return eSceneType::eTitle;

	}
	// ステージ選択の確認の操作
	else
	{
		cur_max = 2;
		cur_min = 1;

		// カーソルを右に移動
		if (input->GetKeyDown(KEY_INPUT_D) || input->GetButtonDown(XINPUT_BUTTON_DPAD_RIGHT))
		{
			if (check_cursor < cur_max)
			{
				check_cursor++;
			}
		}
		// カーソルを左に移動
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
				// 選択したステージの情報を保存する
				SelectStage* stage = Singleton<SelectStage>::GetInstance();
				switch (stage_cursor)
				{
				case 1:
					stage->SetSelectStage(StageID::Stage1);
					break;
				case 2:
					stage->SetSelectStage(StageID::Stage2);
					break;
				case 3:
					stage->SetSelectStage(StageID::Stage3);
					break;
				default:
					stage->SetSelectStage(StageID::Stage1);
					break;
				}

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

	// カーソルを合わせたら動画を再生
	switch (stage_cursor)
	{
	case 1:
		PauseMovieToGraph(stage_movie[1]);
		PauseMovieToGraph(stage_movie[2]);
		PlayMovieToGraph(stage_movie[0]);
		break;
	case 2:
		PauseMovieToGraph(stage_movie[0]);
		PauseMovieToGraph(stage_movie[2]);
		PlayMovieToGraph(stage_movie[1]);
		break;
	case 3:
		PauseMovieToGraph(stage_movie[0]);
		PauseMovieToGraph(stage_movie[1]);
		PlayMovieToGraph(stage_movie[2]);
		break;
	default:
		break;
	}

#if 0
	if (input->GetKeyDown(KEY_INPUT_UP))
		image_y--;
	else if (input->GetKeyDown(KEY_INPUT_DOWN))
		image_y++;
	else if (input->GetKeyDown(KEY_INPUT_LEFT))
		image_x--;
	else if (input->GetKeyDown(KEY_INPUT_RIGHT))
		image_x++;

	if (input->GetKeyDown(KEY_INPUT_M))
		image_scale += 0.1f;
	else if (input->GetKeyDown(KEY_INPUT_N))
		image_scale -= 0.1f;
#endif

	return GetNowSceneType();
}

/// <summary>
/// 描画処理
/// </summary>
/// <returns></returns>
void SelectScene::Draw()
{
	// ステージ画像
	DrawRotaGraph(210, 375, 1.0f, 0.0f, stage_image[0], TRUE);
	DrawRotaGraph(638, 374, 1.0f, 0.0f, stage_image[1], TRUE);
	DrawRotaGraph(1066, 298, 1.0f, 0.0f, stage_image[2], TRUE);

	// 背景の動いている横線
	for (int y = 0; y < D_WIN_MAX_Y; y += 4)
	{
		int bright = 10 + (y + GetNowCount() / 3) % 40;
		DrawLine(0, y, D_WIN_MAX_X, y, GetColor(0, bright, bright));
	}

	// オフセット
	float start_x = -5.0f;
	float start_y = 0.0f;
	float box_offset = 427.0f;
	int select_color = GetColor(255, 255, 255);

	// カーソルの表示（選択したものを明るく）
	switch (stage_cursor)
	{
	case 1:
		// 動画の描画
		DrawExtendGraph(0 - (D_WIN_MAX_X / 3), 0, D_WIN_MAX_X - (D_WIN_MAX_X / 3), D_WIN_MAX_Y, stage_movie[0], FALSE);
		DrawRotaGraph(638, 374, 1.0f, 0.0f, stage_image[1], TRUE);

		// 薄めのカーソルの表示
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawBox(start_x + ((2 * box_offset) - box_offset), 0, box_offset + ((2 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		DrawBox(start_x + ((3 * box_offset) - box_offset), 0, box_offset + ((3 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

		break;
	case 2:
		// 動画の描画
		DrawExtendGraph(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, stage_movie[1], FALSE);
		DrawRotaGraph(210, 375, 1.0f, 0.0f, stage_image[0], TRUE);
		DrawRotaGraph(1066, 298, 1.0f, 0.0f, stage_image[2], TRUE);

		// 薄めのカーソルの表示
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawBox(start_x + ((1 * box_offset) - box_offset), 0, box_offset + ((1 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		DrawBox(start_x + ((3 * box_offset) - box_offset), 0, box_offset + ((3 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

		break;
	case 3:
		// 動画の描画
		DrawExtendGraph(D_WIN_MAX_X / 3, 0, D_WIN_MAX_X + (D_WIN_MAX_X / 3), D_WIN_MAX_Y, stage_movie[2], FALSE);
		DrawRotaGraph(638, 374, 1.0f, 0.0f, stage_image[1], TRUE);

		// 薄めのカーソルの表示
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawBox(start_x + ((1 * box_offset) - box_offset), 0, box_offset + ((1 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		DrawBox(start_x + ((2 * box_offset) - box_offset), 0, box_offset + ((2 * box_offset) - box_offset), D_WIN_MAX_Y,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

		break;
	default:
		DrawString(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2, "Not Select", GetColor(255, 255, 255), TRUE);
		break;
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
	// ベストタイムとハイスコア表示する帯
	DrawBox(0, 580, D_WIN_MAX_X, 670, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	DrawLine(D_WIN_MAX_X / 3, 0, D_WIN_MAX_X / 3, D_WIN_MAX_Y, GetColor(255, 255, 255), 1);
	DrawLine((D_WIN_MAX_X / 3) * 2, 0, (D_WIN_MAX_X / 3) * 2, D_WIN_MAX_Y, GetColor(255, 255, 255), 1);

	// ステージ選択のカーソル
	if (stage_check == false)
	{
		// ステージ名オフセット
		int im_x;
		int im_y;
		// カーソルが指したステージ名表示
		switch (stage_cursor)
		{
		case 1:
			// ステージ名オフセット
			im_x = (D_WIN_MAX_X / 3) - 270;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "stage1", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			DrawString(im_x - 120, im_y + 230, "best time : 2'22'22", GetColor(255, 255, 255), TRUE);
			DrawString(im_x - 120, im_y + 270, "high score : 1,000,000", GetColor(255, 255, 255), TRUE);
			SetFontSize(16);
			break;
		case 2:
			// ステージ名オフセット
			im_x = (D_WIN_MAX_X / 2) - 70;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "stage2", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			DrawString(im_x - 110, im_y + 230, "best time : 2'22'22", GetColor(255, 255, 255), TRUE);
			DrawString(im_x - 110, im_y + 270, "high score : 1,000,000", GetColor(255, 255, 255), TRUE);
			SetFontSize(16);
			break;
		case 3:
			// ステージ名オフセット
			im_x = D_WIN_MAX_X - 270;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "stage3", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			DrawString(im_x - 120, im_y + 230, "best time : 2'22'22", GetColor(255, 255, 255), TRUE);
			DrawString(im_x - 120, im_y + 270, "high score : 1,000,000", GetColor(255, 255, 255), TRUE);
			SetFontSize(16);
			break;
		default:
			DrawString(D_WIN_MAX_X / 2, D_WIN_MAX_Y / 2, "Not Select", GetColor(255, 255, 255), TRUE);
			break;
		}
	}
	// ステージ選択の確認のカーソル
	else
	{
		// 確認文字のラベル
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
		DrawBox(0, D_WIN_MAX_Y / 2 - 30, D_WIN_MAX_X, D_WIN_MAX_Y / 2 + 50,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

		// 確認文字
		SetFontSize(32);
		DrawString((D_WIN_MAX_X / 2) - 150, D_WIN_MAX_Y / 2, "Start this stage?", GetColor(255, 255, 255), TRUE);
		SetFontSize(16);

		// 確認ボックス
		int off_x = (D_WIN_MAX_X / 2) - 170;
		for (int i = 0; i < 2; i++)
		{
			DrawBox(off_x + (i * 200), 430, (150 + off_x + (i * 200)), 480,
				select_color, TRUE);
		}

		// カーソルの表示
		DrawBox(off_x + ((check_cursor * 200) - 200), 430, 150 + off_x + ((check_cursor * 200) - 200), 480,
			GetColor(0, 255, 0), TRUE);
		DrawString(off_x, 430, "back", GetColor(0, 0, 255), TRUE);
		DrawString(off_x + 200, 430, "ok", GetColor(0, 0, 255), TRUE);
	}
	
#if 0
	DrawFormatString(0, 100, GetColor(255, 255, 255), "image_x : %d", image_x, TRUE);
	DrawFormatString(0, 130, GetColor(255, 255, 255), "image_y : %d", image_y, TRUE);
	DrawFormatString(0, 160, GetColor(255, 255, 255), "image_scale : %.1f", image_scale, TRUE);
#endif
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
