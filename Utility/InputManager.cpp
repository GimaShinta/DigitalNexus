#include "InputManager.h"
#include "DxLib.h"

/// <summary>
/// インスタンス削除処理
/// </summary>
void InputManager::DeleteInstance()
{
	// 自クラスのポインタ（実体をアドレスの先で保有）
	static InputManager* instance;
	// インスタンスが存在している場合、削除する
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

/// <summary>
/// 更新処理
/// </summary>
void InputManager::Update()
{
	// キー入力値の更新
	memcpy(old_key, now_key, (sizeof(char) * D_KEYCODE_MAX));
	GetHitKeyStateAll(now_key);

	// コントローラー入力値の更新
	XINPUT_STATE input = {};
	GetJoypadXInputState(DX_INPUT_PAD1, &input);
	for (int i = 0; i < D_BUTTON_MAX; i++)
	{
		old_button[i] = now_button[i];
		now_button[i] = static_cast<bool>(input.Buttons[i]);
	}
	trigger[0] = TriggerNormalization(input.LeftTrigger);
	trigger[1] = TriggerNormalization(input.RightTrigger);
	stick[0].x = StickNormalization(input.ThumbLX);
	stick[0].y = StickNormalization(input.ThumbLY);
	stick[1].x = StickNormalization(input.ThumbRX);
	stick[1].y = StickNormalization(input.ThumbRY);
}

/// <summary>
/// キーボードのキーが押されているか確認する
/// </summary>
/// <param name="key_code">割り当てられたキー</param>
/// <returns>押されていたら、true</returns>
bool InputManager::GetKey(int key_code) const
{
	return CheckKeyCodeRange(key_code) && (now_key[key_code] == TRUE && old_key[key_code] == TRUE);
}

/// <summary>
/// キーボードのキーが押された瞬間か確認する
/// </summary>
/// <param name="key_code">割り当てられたキー</param>
/// <returns>押された瞬間なら、true</returns>
bool InputManager::GetKeyDown(int key_code) const
{
	return CheckKeyCodeRange(key_code) && (now_key[key_code] == TRUE && old_key[key_code] == FALSE);
}

/// <summary>
/// キーボードのキーが離された瞬間か確認する
/// </summary>
/// <param name="key_code">割り当てられたキー</param>
/// <returns>離された瞬間なら、true</returns>
bool InputManager::GetKeyUp(int key_code) const
{
	return CheckKeyCodeRange(key_code) && (now_key[key_code] == FALSE && old_key[key_code] == TRUE);
}

/// <summary>
/// コントローラーのボタンが押されているか確認する
/// </summary>
/// <param name="button">割り当てられたボタン</param>
/// <returns>押されていたら、true</returns>
bool InputManager::GetButton(int button) const
{
	return CheckButtonRange(button) && (now_button[button] && old_button[button]);
}

/// <summary>
/// コントローラーのボタンが押された瞬間か確認する
/// </summary>
/// <param name="button">割り当てられたボタン</param>
/// <returns>押された瞬間なら、true</returns>
bool InputManager::GetButtonDown(int button) const
{
	return CheckButtonRange(button) && (now_button[button] && !old_button[button]);
}

/// <summary>
/// コントローラーのボタンが離された瞬間か確認する
/// </summary>
/// <param name="button">割り当てられたボタン</param>
/// <returns>離された瞬間なら、true</returns>
bool InputManager::GetButtonUp(int button) const
{
	return CheckButtonRange(button) && (!now_button[button] && old_button[button]);
}

/// <summary>
/// 左トリガー入力状態取得
/// </summary>
/// <returns>左トリガー入力状態(0.0f～1.0f)</returns>
float InputManager::GetLeftTrigger() const
{
	return trigger[0];
}

/// <summary>
/// 右トリガー入力状態取得
/// </summary>
/// <returns>右トリガー入力状態(0.0f～1.0f)</returns>
float InputManager::GetRightTrigger() const
{
	return trigger[1];
}

/// <summary>
/// 左スティック入力状態取得
/// </summary>
/// <returns>左スティック入力状態(-1.0f～1.0f)</returns>
Vector2D InputManager::GetLeftStick() const
{
	return stick[0];
}

/// <summary>
/// 右スティック入力状態取得
/// </summary>
/// <returns>右スティック入力状態(-1.0f～1.0f)</returns>
Vector2D InputManager::GetRightStick() const
{
	return stick[1];
}

/// <summary>
/// トリガー入力値の正規化
/// </summary>
/// <param name="value">トリガー入力値</param>
/// <returns>入力値を正規化した値</returns>
float InputManager::TriggerNormalization(unsigned char value)
{
	float GameMain = 0.0f;

	GameMain = (float)value / (float)UCHAR_MAX;

	return GameMain;
}

/// <summary>
/// スティック入力値の正規化
/// </summary>
/// <param name="value">スティック入力値</param>
/// <returns>入力値を正規化した値</returns>
float InputManager::StickNormalization(short value)
{
	float GameMain = 0.0f;

	if (value >= 0)
	{
		GameMain = (float)value / (float)SHRT_MAX;
	}
	else
	{
		GameMain = -((float)value / (float)SHRT_MIN);
	}

	return GameMain;
}

/// <summary>
/// 割り当てキーコード範囲内チェック処理
/// </summary>
/// <param name="button">割り当てられたキーコード</param>
/// <returns>範囲内なら、true</returns>
bool InputManager::CheckKeyCodeRange(int key_code) const
{
	return (0 <= key_code && key_code < D_KEYCODE_MAX);
}

/// <summary>
/// 割り当てボタン範囲内チェック処理
/// </summary>
/// <param name="buttton">割り当てられたボタン</param>
/// <returns>範囲内なら、true</returns>
bool InputManager::CheckButtonRange(int button) const
{
	return (0 <= button && button < D_BUTTON_MAX);
}
