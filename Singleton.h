#pragma once

// シングルトンテンプレートクラス
template <typename T>
class Singleton
{
protected:

	// クラスの実体をメンバ関数内でしか生成できないようにする
	Singleton() = default;

	// コピーガード
	Singleton(const Singleton&) = delete;
	Singleton& operator = (const Singleton&) = delete;
	// ~コピーガード

	~Singleton() = default;

public:
	// インスタンスの取得
	static T* GetInstance();
};

// インスタンスの取得
template<typename T>
inline T* Singleton<T>::GetInstance()
{
	// static 変数としてインスタンスを保持
	// T* ではなく T 型のインスタンスを直接保持する
	static T instance;
	// アドレスを返す
	return &instance;
}