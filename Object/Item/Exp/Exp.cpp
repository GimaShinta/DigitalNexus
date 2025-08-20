#include "Exp.h"
#include "DxLib.h"
#include "../../Character/Player/Player.h"
#include "../../GameObjectManager.h"
#include "../../../Utility/ScoreData.h"

Exp::Exp() :
    lifetime(5.0f)         //自動削除する秒数
{
}

Exp::~Exp()
{
}

// 初期化処理
void Exp::Initialize()
{
    // 当たり判定設定
    collision.is_blocking = true;                         // 接触判定有効
    collision.object_type = eObjectType::eExp;            // 自分の種類はExp
    collision.hit_object_type.push_back(eObjectType::ePlayer); // プレイヤーと衝突判定

    box_size = 6.0f;             // 経験値アイテムのサイズ（半径）
    z_layer = 1;                 // 描画レイヤー（前面に表示）

    is_mobility = true;          // 当たり判定で移動判定有効
    is_attracting = false;       // 吸収モードOFFで開始
    velocity = Vector2D(0, 50.0f); // 初期落下（ふわっと下へ）

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();

    images = rm->GetImages("Resource/Image/Effect/Exp/exp_s/pipo-nazoobj03b_192.png", 30, 5, 6, 192, 192);
    image = images[0];
}

// 更新処理
void Exp::Update(float delta)
{
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

    // 寿命減少（自動消滅処理）
    lifetime -= delta;
    if (lifetime <= 0.0f)
    {
        this->SetDestroy(); // 5秒後に削除
        return;
    }

    // 画面外に落ちたら削除
    if (location.y > D_WIN_MAX_Y + 200)
    {
        this->SetDestroy();
        return;
    }

    std::vector<int> animation_num = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
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

}

// 描画処理
void Exp::Draw(const Vector2D& offset) const
{
    DrawRotaGraph(location.x, location.y, 1.5f, 0.0f, image, TRUE);
}

// 衝突処理
void Exp::OnHitCollision(GameObjectBase* hit_object)
{
    // すでに吸収済みなら無視
    if (is_collected) return;

    // プレイヤーと当たったら
    if (hit_object->GetCollision().object_type == eObjectType::ePlayer)
    {
        is_collected = true;

        SEManager::GetInstance()->PlaySE(SE_NAME::Get);
        SEManager::GetInstance()->ChangeSEVolume(SE_NAME::Get, 80);

        if (player && !player->GetBeamOn())  // ビーム中でなければ加算
        {
            player->AddCharge(1.0f);  // 調整可能
        }
        Singleton<ScoreData>::GetInstance()->AddScore(100);
        this->SetDestroy(); // 吸収 → 削除
    }
}
