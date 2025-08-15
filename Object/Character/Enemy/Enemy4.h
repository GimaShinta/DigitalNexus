#pragma once
#include "EnemyBase.h"
#include <vector>

enum class Enemy4Mode {
    Zako3,   // Enemy3(Zako3)互換
};

enum class Enemy4State {
    Appearing,   // せり出し（手前へ寄る演出）
    Floating,    // 小さく揺れて滞空
    Leaving,     // 上へ退場
};

class Enemy4 : public EnemyBase {
public:
    Enemy4();
    ~Enemy4();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void SetAppearParams(const Vector2D& start_pos_param,
        const Vector2D& end_pos_param,
        float appear_time,
        bool from_left);
    void SetMode(Enemy4Mode m) { mode = m; }
    void SetFlip(bool flip) { zako3_flip = flip; }

    // Zako3と同じ“プレイヤーへ一発”
    void ShootToPlayer(float speed);

private:
    // モード/ステート
    Enemy4Mode  mode = Enemy4Mode::Zako3;
    Enemy4State state = Enemy4State::Appearing;

    // 時間系
    float float_timer = 0.0f; // 浮遊・生存カウント
    float appear_timer = 0.0f; // 登場アニメ
    float appear_duration = 1.2f; // 到達まで

    // 見た目
    float scale_min = 0.3f;
    float scale_max = 0.8f;
    float scale = 0.3f;
    float rotation = 0.0f;
    int   alpha = 0;
    bool  zako3_flip = false;
    bool  is_from_left = true;

    // 位置
    Vector2D start_pos = 0.0f;
    Vector2D target_pos = 0.0f;
    Vector2D base_loc = 0.0f;

    // 画像
    std::vector<int> zako3_images;
    int zako3_image = -1;

    template <typename T> T my_min(T a, T b) { return (a < b) ? a : b; }
};
