#include "MaxCharge.h"
#include "../../../Utility/ScoreData.h"

void MaxCharge::Initialize()
{
    velocity.y = 60.0f;
    box_size = 20.0f;
    z_layer = 1;
    is_mobility = true;

    // 当たり判定のオブジェクト設定
    collision.is_blocking = true;
    // 自分のオブジェクトタイプ
    collision.object_type = eObjectType::eMaxCharge;
    // 当たる相手のオブジェクトタイプ
    collision.hit_object_type.push_back(eObjectType::ePlayer);

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    images = rm->GetImages("Resource/Image/Object/Item/PowerUp/max.png", 48, 8, 6, 48, 64);
    image = images[0];

    se = rm->GetSounds("Resource/sound/se/effect/audiostock_1133382.mp3");
    ChangeVolumeSoundMem(255 * 100 / 100, se);
}

void MaxCharge::Update(float delta)
{
    std::vector<int> animation_num = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 };
    //フレームレートで時間を計測
    animation_time += delta;
    //8秒経ったら画像を切り替える
    if (animation_time >= 0.03f)
    {
        //計測時間の初期化
        animation_time = 0.0f;
        //時間経過カウントの増加
        animation_count++;
        //カウントがアニメーション画像の要素数以上になったら
        if (animation_count >= animation_num.size())
        {
            //カウントの初期化
            animation_count = 0;
        }
        // アニメーションが順番に代入される
        image = images[animation_num[animation_count]];
    }


    if (!player)
    {
        return; // プレイヤーが未設定なら処理しない
    }

    //プレイヤー情報：位置情報
    Vector2D to_player = player->GetLocation() - location;

    // 距離の2乗で吸収開始を判定
    float dist_sq = to_player.x * to_player.x + to_player.y * to_player.y;
    float attract_range_sq = attract_range * attract_range;

    if (!is_attracting && dist_sq < attract_range_sq)
    {
        // 吸収開始
        is_attracting = true;
    }

    if (is_attracting)
    {
        // 吸収モード中：加速してプレイヤーに向かう
        to_player.Normalize();

        speed += acceleration * delta;   // 徐々に加速

        if (speed > max_speed)
        {
            speed = max_speed; // 最大スピード制限
        }
        velocity = to_player * speed;
    }

    // 現在の速度で位置更新
    location += velocity * delta;

}

void MaxCharge::Draw(const Vector2D& offset) const
{
    DrawRotaGraph(location.x, location.y, 1.0f, 0.0f, image, TRUE);

}

void MaxCharge::OnHitCollision(GameObjectBase* hit_object)
{
    if (hit_object->GetCollision().object_type == eObjectType::ePlayer)
    {
        PlaySoundMem(se, DX_PLAYTYPE_BACK);
        Singleton<ScoreData>::GetInstance()->AddScore(GetRand(100) + 50);
        this->SetDestroy();
    }

}
