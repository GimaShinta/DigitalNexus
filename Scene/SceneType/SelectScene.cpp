#include "SelectScene.h"
#include "../../Utility/SelectStage.h"
#include "../../Utility/ScoreData.h"

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

	bgm = rm->GetSounds("Resource/sound/se/se_effect/select_01.mp3");
	cursor_se = rm->GetSounds("Resource/sound/se/se_effect/cursor_01.mp3");
	tap_se = rm->GetSounds("Resource/sound/se/se_effect/start_01.mp3");

	ChangeVolumeSoundMem(255 * 40 / 100, bgm);
	PlaySoundMem(bgm, DX_PLAYTYPE_LOOP);

	// AnimationManager による事前アニメ再生（0.01秒、画面外で一度だけ再生）
	EffectManager* effe = Singleton<EffectManager>::GetInstance();
	SEManager* se = Singleton<SEManager>::GetInstance();

	effe->LoadAllEffects();
	se->LoadSE();
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
				PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
			}
		}
		// カーソルを左に移動
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_DPAD_LEFT))
		{
			if (stage_cursor > cur_min)
			{
				stage_cursor--;
				PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
			}
		}

		// ステージの選択をする
		if (input->GetKeyDown(KEY_INPUT_SPACE) || input->GetButtonDown(XINPUT_BUTTON_A)) {
			stage_check = true;
			PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
		}

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
				PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
				check_cursor++;
			}
		}
		// カーソルを左に移動
		if (input->GetKeyDown(KEY_INPUT_A) || input->GetButtonDown(XINPUT_BUTTON_DPAD_LEFT))
		{
			if (check_cursor > cur_min)
			{
				PlaySoundMem(cursor_se, DX_PLAYTYPE_BACK);
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
				PlaySoundMem(tap_se, DX_PLAYTYPE_BACK);
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

				stage_decide_effect = true;
				stage_decide_timer = 0.0f;
				PlaySoundMem(tap_se, DX_PLAYTYPE_BACK);

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

	// ===== ステージ決定演出中 =====
	if (stage_decide_effect)
	{
		stage_decide_timer += delta_second;

		// 0.45秒後にゲーム開始
		if (stage_decide_timer >= 0.45f)
		{
			return eSceneType::eGameMain;
		}
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
	DrawBox(0, 600, D_WIN_MAX_X, 670, GetColor(0, 0, 0), TRUE);
	DrawBox(0, 350, D_WIN_MAX_X, 400, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

	DrawLine(D_WIN_MAX_X / 3, 0, D_WIN_MAX_X / 3, D_WIN_MAX_Y, GetColor(255, 255, 255), 1);
	DrawLine((D_WIN_MAX_X / 3) * 2, 0, (D_WIN_MAX_X / 3) * 2, D_WIN_MAX_Y, GetColor(255, 255, 255), 1);

	// ステージ選択のカーソル
	if (stage_check == false)
	{
		// ステージ名オフセット
		int im_x;
		int im_y;

		ScoreData* score = Singleton<ScoreData>::GetInstance();
		Time time;
		// カーソルが指したステージ名表示
		switch (stage_cursor)
		{
		case 1:
			time = score->GetTime(1);
			// ステージ名オフセット
			im_x = (D_WIN_MAX_X / 3) - 270;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "STAGE 1", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			//DrawFormatString(im_x - 120, im_y + 230, GetColor(255, 255, 255), "best time : %.0f'%.0f'%.0f", time.time_hun, time.time_byou, time.time_miri, TRUE);
			DrawFormatString(im_x - 120, im_y + 270, GetColor(255, 220, 80), "HIGH SCORE : %.0f", score->GetStageScore(1), TRUE);
			SetFontSize(16);
			break;
		case 2:
			time = score->GetTime(2);
			// ステージ名オフセット
			im_x = (D_WIN_MAX_X / 2) - 70;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "STAGE 2", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			//DrawFormatString(im_x - 110, im_y + 230, GetColor(255, 255, 255), "best time : %.0f'%.0f'%.0f", time.time_hun, time.time_byou, time.time_miri, TRUE);
			DrawFormatString(im_x - 110, im_y + 270, GetColor(255, 220, 80), "HIGH SCORE : %.0f", score->GetStageScore(2), TRUE);
			SetFontSize(16);
			break;
		case 3:
			time = score->GetTime(3);
			// ステージ名オフセット
			im_x = D_WIN_MAX_X - 270;
			im_y = D_WIN_MAX_Y / 2;
			// ステージ名
			SetFontSize(32);
			DrawString(im_x, im_y, "STAGE 3", GetColor(255, 255, 255), TRUE);
			SetFontSize(24);
			//DrawFormatString(im_x - 120, im_y + 230, GetColor(255, 255, 255), "best time : %.0f'%.0f'%.0f", time.time_hun, time.time_byou, time.time_miri, TRUE);
			DrawFormatString(im_x - 120, im_y + 270, GetColor(255, 220, 80), "HIGH SCORE : %.0f", score->GetStageScore(3), TRUE);
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
		// ===== 確認ウィンドウ（サイバー風） =====
		int t = GetNowCount();
		float pulse = (sinf(t / 120.0f) + 1.0f) * 0.5f;

		// 背景パネル
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
		DrawBox(0, D_WIN_MAX_Y / 2 - 80, D_WIN_MAX_X, D_WIN_MAX_Y / 2 + 120,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 外枠ネオン
		int frameCol = GetColor(0, 220, 255);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120 + (int)(80 * pulse));
		DrawBox(200, D_WIN_MAX_Y / 2 - 80, D_WIN_MAX_X - 200, D_WIN_MAX_Y / 2 - 76, frameCol, TRUE);
		DrawBox(200, D_WIN_MAX_Y / 2 + 116, D_WIN_MAX_X - 200, D_WIN_MAX_Y / 2 + 120, frameCol, TRUE);
		DrawBox(200, D_WIN_MAX_Y / 2 - 80, 204, D_WIN_MAX_Y / 2 + 120, frameCol, TRUE);
		DrawBox(D_WIN_MAX_X - 204, D_WIN_MAX_Y / 2 - 80, D_WIN_MAX_X - 200, D_WIN_MAX_Y / 2 + 120, frameCol, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// タイトル
		SetFontSize(36);

		// 影
		DrawString(D_WIN_MAX_X / 2 - 210 + 2, D_WIN_MAX_Y / 2 - 50 + 2,
			"START THIS STAGE ?", GetColor(0, 0, 0), TRUE);

		// グロー
		SetDrawBlendMode(DX_BLENDMODE_ADD, 120 + (int)(80 * pulse));
		DrawString(D_WIN_MAX_X / 2 - 210, D_WIN_MAX_Y / 2 - 50,
			"START THIS STAGE ?", GetColor(120, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 下線
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
		DrawLine(D_WIN_MAX_X / 2 - 210, D_WIN_MAX_Y / 2 - 18,
			D_WIN_MAX_X / 2 + 210, D_WIN_MAX_Y / 2 - 18,
			GetColor(0, 200, 255));
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		SetFontSize(16);

		// ===== YES / NO ボタン =====
		for (int i = 0; i < 2; ++i)
		{
			int off_x = (D_WIN_MAX_X / 2) - 170;
			bool selected = (check_cursor == i + 1);

			int bx = off_x + i * 200;
			int by = 430;
			int bw = 150;
			int bh = 50;

			// ボタン背景
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, selected ? 160 : 90);
			DrawBox(bx, by, bx + bw, by + bh, GetColor(0, 0, 0), TRUE);

			// 枠
			int col = selected ? GetColor(0, 220, 255) : GetColor(80, 120, 140);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, selected ? 200 : 100);
			DrawBox(bx, by, bx + bw, by + 2, col, TRUE);
			DrawBox(bx, by + bh - 2, bx + bw, by + bh, col, TRUE);
			DrawBox(bx, by, bx + 2, by + bh, col, TRUE);
			DrawBox(bx + bw - 2, by, bx + bw, by + bh, col, TRUE);

			// 選択中スキャンライン
			if (selected)
			{
				int sy = by + (t / 3) % bh;
				SetDrawBlendMode(DX_BLENDMODE_ADD, 80);
				DrawBox(bx + 4, sy, bx + bw - 4, sy + 2, GetColor(120, 255, 255), TRUE);
			}

			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

			// テキスト
			const char* txt = (i == 0) ? "NO" : "YES";
			SetFontSize(24);

			// 影
			DrawString(bx + 45 + 1, by + 12 + 1, txt, GetColor(0, 0, 0), TRUE);

			// 本体
			DrawString(bx + 45, by + 12,
				txt,
				selected ? GetColor(255, 255, 255) : GetColor(180, 180, 180),
				TRUE);
		}

		SetFontSize(16);

	}
	// ===== ステージ決定：グリッチ遷移 =====
	if (stage_decide_effect)
	{
		float tt = stage_decide_timer; // 0～0.45
		float k = tt / 0.45f;
		if (k > 1.0f) k = 1.0f;

		// 画面の横ズレ（最初強く→だんだん弱く）
		int jitter = (int)((1.0f - k) * 18.0f);
		int ox = (GetRand(jitter * 2 + 1) - jitter);
		int oy = (GetRand(5) - 2);

		// 黒フェード（最後に暗転へ寄せる）
		int fadeA = (int)(255 * k);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeA);
		DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// ノイズ帯（横長の矩形をランダムに）
		int bandCount = 22;
		for (int i = 0; i < bandCount; ++i)
		{
			int y = GetRand(D_WIN_MAX_Y);
			int h = 2 + GetRand(10);
			int w = 80 + GetRand(520);
			int x = GetRand(D_WIN_MAX_X) + ox; // 横ズレ反映
			int a = 40 + GetRand(120);

			SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
			DrawBox(x, y + oy, x + w, y + h + oy, GetColor(255, 255, 255), TRUE);
		}

		// 走査線（薄い横線を数本）
		for (int i = 0; i < 10; ++i)
		{
			int y = (int)(D_WIN_MAX_Y * k) + GetRand(260) - 130;
			if (y < 0) y = 0;
			if (y > D_WIN_MAX_Y) y = D_WIN_MAX_Y;

			int a = 30 + GetRand(80);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
			DrawLine(0 + ox, y + oy, D_WIN_MAX_X + ox, y + oy, GetColor(120, 255, 255));
		}

		// 一瞬だけ強いフラッシュ（開始直後の“ビッ”）
		if (tt < 0.08f)
		{
			int a = (int)(255 * (1.0f - tt / 0.08f));
			SetDrawBlendMode(DX_BLENDMODE_ADD, a);
			DrawBox(0, 0, D_WIN_MAX_X, D_WIN_MAX_Y, GetColor(255, 255, 255), TRUE);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
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
	StopSoundMem(bgm);
}

/// <summary>
/// 現在のシーン情報
/// </summary>
/// <returns>現在はリザルトシーンです</returns>
eSceneType SelectScene::GetNowSceneType() const
{
	return eSceneType::eSelect;
}
