#pragma once
#include "EnemyBase.h"

enum class ZakoMode {
    Zako2,
    Zako3,
    Zako7
};

enum class ZakoState {
    AppearingFromBottom,  // Zako2スタイル
    Descending,           // Zako2スタイル
    Appearing,            // Zako3スタイル
    Floating,              // 共通
    Leaving,
    Redirecting,
};

class Enemy3 : public EnemyBase {
public:
    Enemy3();
    ~Enemy3();

    void Initialize() override;
    void Update(float delta_second) override;
    void Draw(const Vector2D& screen_offset) const override;
    void Finalize() override;

    void Shot(float delta_second);
    void SetFlip(bool flip);

    /// 出現アニメーション用パラメータの設定
    void SetAppearParams(const Vector2D& start_pos_param, const Vector2D& end_pos_param, float appear_time, bool from_left);

    /// Zako2 or Zako3 の動作切り替え
    void SetMode(ZakoMode mode);
    void ShootToPlayer(float speed);
private:
    ZakoMode mode;
    ZakoState state;

    float float_timer = 0.0f;
    float appear_timer = 0.0f;
    float appear_duration = 0.0f;
    float descend_duration = 0.0f;
    int  sound_destroy = NULL;

    float scale_min = 0.0f;
    float scale_max = 0.0f;

    float alpha = 0.0f;
    float scale = 0.0f;
    float rotation = 0.0f;

    bool is_from_left = false;
    bool zako3_flip = false;

    Vector2D start_pos = 0.0f;
    Vector2D target_pos = 0.0f;
    Vector2D descend_pos = 0.0f;
    Vector2D base_location = 0.0f;

    std::vector<int> zako2_images;
    std::vector<int> zako3_images;

    int zako2_image = NULL;
    int zako3_image = NULL;
    int zako7_image = NULL;
    int zako8_image = NULL;

    std::vector<int> enemy_jet;
    int jet = NULL;
    float jet_time = 0.0f;
    int jet_count = 0;

    template <typename T>
    T my_min(T a, T b) {
        return (a < b) ? a : b;
    }

};