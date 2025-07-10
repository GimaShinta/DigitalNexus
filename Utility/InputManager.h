#pragma once

#include "Vector2D.h"
#include"../Singleton.h"

// マクロ定義
#define	D_KEYCODE_MAX	(256)		// キーボード入力値最大数
#define D_BUTTON_MAX	(16)		// コントローラーの入力ボタン最大数

/// <summary>
/// 入力管理クラス（Singleton）
/// </summary>
class InputManager : public Singleton<InputManager>
{
private:
	// キー入力情報
	char now_key[D_KEYCODE_MAX];		// 現在の入力値
	char old_key[D_KEYCODE_MAX];		// 前回の入力値

	// コントローラー入力情報
	bool now_button[D_BUTTON_MAX];		// 現在のボタン入力値
	bool old_button[D_BUTTON_MAX];		// 前回のボタン入力値
	float trigger[2];					// トリガー入力値（0.0f～1.0f）
	Vector2D stick[2];					// スティック入力値（-1.0f～1.0f）

public:

	/// <summary>
	/// インスタンス削除処理
	/// </summary>
	static void DeleteInstance();

public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

public:
	// 入力状態の取得

	/// <summary>
	/// キーボードのキーが押されているか確認する
	/// </summary>
	/// <param name="key_code">割り当てられたキー</param>
	/// <returns>押されていたら、true</returns>
	bool GetKey(int key_code) const;

	/// <summary>
	/// キーボードのキーが押された瞬間か確認する
	/// </summary>
	/// <param name="key_code">割り当てられたキー</param>
	/// <returns>押された瞬間なら、true</returns>
	bool GetKeyDown(int key_code) const;

	/// <summary>
	/// キーボードのキーが離された瞬間か確認する
	/// </summary>
	/// <param name="key_code">割り当てられたキー</param>
	/// <returns>離された瞬間なら、true</returns>
	bool GetKeyUp(int key_code) const;

	/// <summary>
	/// コントローラーのボタンが押されているか確認する
	/// </summary>
	/// <param name="button">割り当てられたボタン</param>
	/// <returns>押されていたら、true</returns>
	bool GetButton(int button) const;
	/// <summary>
	/// コントローラーのボタンが押された瞬間か確認する
	/// </summary>
	/// <param name="button">割り当てられたボタン</param>
	/// <returns>押された瞬間なら、true</returns>
	bool GetButtonDown(int button) const;
	/// <summary>
	/// コントローラーのボタンが離された瞬間か確認する
	/// </summary>
	/// <param name="button">割り当てられたボタン</param>
	/// <returns>離された瞬間なら、true</returns>
	bool GetButtonUp(int button) const;

	/// <summary>
	/// 左トリガー入力状態取得
	/// </summary>
	/// <returns>左トリガー入力状態(0.0f～1.0f)</returns>
	float GetLeftTrigger() const;

	/// <summary>
	/// 右トリガー入力状態取得
	/// </summary>
	/// <returns>右トリガー入力状態(0.0f～1.0f)</returns>
	float GetRightTrigger() const;

	/// <summary>
	/// 左スティック入力状態取得
	/// </summary>
	/// <returns>左スティック入力状態(-1.0f～1.0f)</returns>
	Vector2D GetLeftStick() const;

	/// <summary>
	/// 右スティック入力状態取得
	/// </summary>
	/// <returns>右スティック入力状態(-1.0f～1.0f)</returns>
	Vector2D GetRightStick() const;

private:
	/// <summary>
	/// トリガー入力値の正規化
	/// </summary>
	/// <param name="value">トリガー入力値</param>
	/// <returns>入力値を正規化した値</returns>
	float TriggerNormalization(unsigned char value);

	/// <summary>
	/// スティック入力値の正規化
	/// </summary>
	/// <param name="value">スティック入力値</param>
	/// <returns>入力値を正規化した値</returns>
	float StickNormalization(short value);

	/// <summary>
	/// 割り当てキーコード範囲内チェック処理
	/// </summary>
	/// <param name="key_code">割り当てられたキーコード</param>
	/// <returns>範囲内なら、true</returns>
	bool CheckKeyCodeRange(int key_code) const;

	/// <summary>
	/// 割り当てボタン範囲内チェック処理
	/// </summary>
	/// <param name="buttton">割り当てられたボタン</param>
	/// <returns>範囲内なら、true</returns>
	bool CheckButtonRange(int button) const;
};