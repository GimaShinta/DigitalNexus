#pragma once
#include "EnemyBase.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/SEManager.h"
#include "../../../Object/GameObjectManager.h"

enum class Enemy5State {
    Appearing,
    Moving,
    Leaving,
};

class Enemy5 : public EnemyBase {
public:
    Enemy5();
    Enemy5(const Vector2D& pos);
    ~Enemy5();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetPlayer(Player* p) { player = p; }

    // 出現開始位置とフェードイン時間
    void SetAppearParams(const Vector2D& start, float appear_time);

    // フォーメーション設定：
    //   index … 自分の隊列内インデックス（0?）
    //   total … 隊列総数
    //   from_left … 左上→右下 なら true / 右上→左下 なら false
    //   spacing … 各機体のオフセット距離（px）
    //   speed   … 移動速度（px/s）
    void SetFormationParams(int index, int total, bool from_left,
        float spacing = 36.0f, float speed = 140.0f,
        int col_idx = 0, int col_total = 1, float col_stride = 28.0f);

private:
    Enemy5State state = Enemy5State::Appearing;

    // 出現
    Vector2D start_pos = 0.0f;
    float appear_duration = 1.0f;
    float appear_timer = 0.0f;
    float scale_min = 1.0f;
    float scale_max = 1.6f;
    float scale = 1.0f;
    int   alpha = 0;

    // 隊列/移動
    bool  from_left_up = true;     // true: 左上→右下 / false: 右上→左下
    int   line_index = 0;
    int   line_total = 1;
    float spacing_px = 36.0f;
    float move_speed = 140.0f;     // 対角等速
    Vector2D velocity = 0.0f;
    int   column_index = 0;      // 0..(column_total-1)
    int   column_total = 1;
    float column_stride = 28.0f; // 列間隔（斜めに対して直角方向）


    // 画像・アニメ
    std::vector<int> images;
    int   image = -1;
    float animation_time = 0.0f;
    int   animation_index = 0;


};
