#pragma once
#include "../StageBase.h"
class Stage1 :
    public StageBase
{
private:
	// 各種攻撃・敵キャラクターのポインタ定義
	class Zako* zako = nullptr;             // 雑魚敵その1
	class EnemyShot1* e_shot1 = nullptr;      // 敵のショットタイプ1
	class Boss1* boss = nullptr;

	float enemy_spawn_timer = 0.0f;     // 敵出現タイマー
	bool zako_spawned = false;         // Zakoがすでに出現したかどうか

	int font_orbitron;
	int font_warning;
public:
	// コンストラクタ・デストラクタ
	Stage1(Player* player);     // プレイヤー情報を引数に取るステージ初期化
	~Stage1();                  // デストラクタ

	// 基本的なステージ処理群
	void Initialize() override;              // ステージ初期化処理
	void Finalize() override;                // ステージ終了処理
	void Update(float delta_second) override;       // 毎フレームの更新処理
	void Draw() override;                    // 描画処理
	StageBase* GetNextStage(Player* player) override; // 次のステージを取得

private:
	void DrawScrollBackground() const override; // 背景のスクロール描画
	void EnemyAppearance(float delta_second) override;
	void UpdateGameStatus(float delta_second) override;
};

