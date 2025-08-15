#pragma once
#include "EnemyBase.h"
#include "../../../Object/Bullet/EnemyBullet/EnemyBullet1.h"

enum class Enemy2Mode {
    Zako3Like,  // 既存：下から弧を描いて到達→射撃→浮遊→退場
    LineRise,   // 追加：列になって順番に下から登場
};

enum class Enemy2State {
    Waiting,    // LineRise用：透明で待機（順番待ち）
    Appearing,  // 出現アニメ（Zako3相当 / LineRise共通）
    Floating,   // ふわふわ滞在
    Leaving,    // 退場
};

class Enemy2 : public EnemyBase {
public:
    Enemy2();
    ~Enemy2();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    // 既存API：位置と時間（appear_timeは今回ちゃんと使用）
    void SetAppearParams(const Vector2D& start_pos_param, const Vector2D& end_pos_param, float appear_time, bool from_left);

    // 追加：モード切替＆ライン演出用パラメータ
    void SetMode(Enemy2Mode m) { mode = m; }
    void SetLineParams(int index, int total, float interval_sec);

    // オプション
    void SetFlip(bool flip) { zako3_flip = flip; }
    void ShootToPlayer(float speed);

private:
    // モードと状態
    Enemy2Mode  mode = Enemy2Mode::Zako3Like;
    Enemy2State state = Enemy2State::Appearing;

    // タイマ類
    float float_timer = 0.0f;       // 浮遊・共通
    float appear_timer = 0.0f;      // 出現アニメ用
    float appear_duration = 1.2f;   // 出現時間（既定を1.2sに）
    float wait_timer = 0.0f;        // LineRise待機タイマ
    float wait_time = 0.0f;         // LineRiseでの自分の開始遅延

    // 見た目
    float scale_min = 0.3f;
    float scale_max = 0.8f;
    float scale = 0.3f;
    float rotation = 0.0f;
    int   alpha = 0;

    // 配置
    bool  is_from_left = true;
    bool  zako3_flip = false;
    Vector2D start_pos = 0.0f;
    Vector2D target_pos = 0.0f;
    Vector2D base_location = 0.0f;

    // ライン登場情報（必要に応じて利用）
    int   line_index = 0;           // 自分の順番（0-based）
    int   line_total = 1;           // 総数
    float line_interval = 0.5f;     // 1体ずつの開始間隔

    // 画像（Zako3系）
    std::vector<int> zako3_images;
    int zako3_image = -1;

    // 内部
    void Shot(float delta_second);
    template <typename T> T my_min(T a, T b) { return (a < b) ? a : b; }
};
