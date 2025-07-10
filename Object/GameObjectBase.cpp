#include "GameObjectBase.h"

// Singleton継承クラスのインクルード
#include "GameObjectManager.h"
#include "../Application/Application.h"

GameObjectBase::GameObjectBase() :
	owner_scene(nullptr),
	location(0.0f),
	image(NULL),
	z_layer(0),
	is_mobility(false),
	is_destroy(false),
	animation_count(0),
	animation_time(0.0f),
	box_size(0.0f),
	score(0),
	velocity(0)
{
}

GameObjectBase::~GameObjectBase()
{
}

// 初期化処理
void GameObjectBase::Initialize()
{
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">１フレーム当たりの時間</param>
void GameObjectBase::Update(float delata_second)
{
	// ウィンドウ外オブジェクトの削除
	GameObjectManager* objm = Singleton<GameObjectManager>::GetInstance();
	if (this->location.x <= 0 - D_OBJECT_SIZE ||
		this->location.x >= D_WIN_MAX_X + D_OBJECT_SIZE ||
		this->location.y >= D_WIN_MAX_Y + 300)
	{
		this->is_destroy = true;
	}
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="delta_second"></param>
/// <returns></returns>
void GameObjectBase::Draw(const Vector2D& screen_offset) const
{
	// オフセット値を基に画像の描画を行う
	Vector2D graph_location = this->location + screen_offset;
	DrawRotaGraphF(graph_location.x, graph_location.y, 1.5, 0.0, image, TRUE);
}

// 終了時処理（使ったインスタンスなどの削除）
void GameObjectBase::Finalize()
{
	GameObjectManager::DeleteInstance();
}

/// <summary>
/// ヒットしたときの処理
/// </summary>
/// <param name="hit_object">プレイヤーだとしたら、ここは敵とかブロックと当たったことになる</param>
void GameObjectBase::OnHitCollision(GameObjectBase* hit_object)
{
}

/// <summary>
/// アニメーション制御
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <param name="animation_image">アニメーション総画像</param>
/// <param name="animation_num">アニメーション順序</param>
/// <param name="animation_num">アニメーション切り替え時間</param>
void GameObjectBase::AnimationControl(float delta_second, std::vector<int>& animation_image, std::vector<int> animation_num, float chenge_time)
{
	//フレームレートで時間を計測
	animation_time += delta_second;
	//8秒経ったら画像を切り替える
	if (animation_time >= (1.0f / chenge_time))
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
		image = animation_image[animation_num[animation_count]];
	}
}

/// <summary>
/// 画像の設定
/// </summary>
/// <param name="delta_second">１フレーム当たりの時間</param>
/// <param name="animation_imag">描画したい単体画像</param>
void GameObjectBase::AnimationControl(int animation_imag)
{
	image = animation_imag;
}

void GameObjectBase::SetOwnerScene(GameObjectManager* scene)
{
	this->owner_scene = scene;
}

/// <summary>
/// 位置座標の設定
/// </summary>
/// <param name="location">設定したい位置</param>
void GameObjectBase::SetLocation(const Vector2D& location)
{
	this->location = location;
	this->old_location = location;
}

/// <summary>
/// 現在の位置座標を取得する
/// </summary>
/// <returns>現在位置</returns>
Vector2D& GameObjectBase::GetLocation()
{
	return location;
}

Vector2D& GameObjectBase::GetVelocity()
{
	return velocity;
}

void GameObjectBase::SetVelocity(const Vector2D& velo)
{
	this->velocity = velo;
}

void GameObjectBase::SetScroll(float scroll)
{
	this->location.x = scroll;
}

void GameObjectBase::SetDestroy()
{
	this->is_destroy = true;
}

/// <summary>
/// オブジェクトサイズの取得
/// </summary>
/// <returns>オブジェクトのサイズ</returns>
Vector2D& GameObjectBase::GetBoxSize()
{
	return box_size;
}

/// <summary>
/// オブジェクトがもってるコリジョン情報（左上と右下の点とか）の取得
/// </summary>
/// <returns>オブジェクトがもってるコリジョン情報（左上と右下の点とか）</returns>
const BoxCollision& GameObjectBase::GetCollision() const
{
	return collision;
}

const unsigned char GameObjectBase::GetZLayer() const
{
	return z_layer;
}

// 動き
const bool& GameObjectBase::GetMobility() const
{
	return is_mobility;
}

const bool& GameObjectBase::Getblokking() const
{
	return this->collision.is_blocking;
}

int GameObjectBase::GetScore()
{
	return score;
}
