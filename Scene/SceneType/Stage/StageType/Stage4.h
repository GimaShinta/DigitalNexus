#pragma once
#include "../StageBase.h"
class Stage4 :
	public StageBase
{
private:
	// 各種攻撃・敵キャラクターのポインタ定義
	class Zako* zako = nullptr;             // 雑魚敵その1
	class Boss4* boss = nullptr;
	class EnemyShot1* e_shot1 = nullptr;      // 敵のショットタイプ1

	bool boss4_spawned = false;
	bool draw_animation_first = false;  // デフォルトはfalse

public:
	// コンストラクタ・デストラクタ
	Stage4(Player* player);     // プレイヤー情報を引数に取るステージ初期化
	~Stage4();                  // デストラクタ

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

