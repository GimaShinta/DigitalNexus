#include "EnemyBullet6.h"
#include "../../Character/Player/Player.h"

EnemyBullet6::EnemyBullet6() : player(nullptr)
{
}

EnemyBullet6::~EnemyBullet6()
{
}

void EnemyBullet6::Initialize()
{
	z_layer = 4;
	box_size = 12;

	// 当たり判定のオブジェクト設定
	collision.is_blocking = true;
	// 自分のオブジェクトタイプ
	collision.object_type = eObjectType::eEnemyShot;
	// 当たる相手のオブジェクトタイプ
	collision.hit_object_type.push_back(eObjectType::ePlayer);
	collision.hit_object_type.push_back(eObjectType::eDefenceShot);

	// 動くかどうか（trueなら動く、falseなら止まる）
	is_mobility = true;

	ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
	image = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange_p/standard/anime74.png", 4, 4, 1, 48, 48)[0];

}

void EnemyBullet6::Update(float delta_second)
{
	if (location.y < -50.0f ||
		location.y > D_WIN_MAX_Y + 50.0f ||
		location.x > 990.0f ||
		location.x < 240.0f)
	{
		is_destroy = true;
	}

	timer += delta_second;

	switch (phase) {
	case MoveIn: {
		// 経過時間に応じてイージング（減速）
		float t = Clamp(timer / total_move_duration, 0.0f, 1.0f);
		float eased = 1.0f - pow(1.0f - t, 2); // easeOutQuad

		Vector2D to_target = target_stop_position - this->location;
		Vector2D move_direction = to_target.Normalize();
		float distance = (target_stop_position - location).Length();

		// 初期速度から0に減速していく
		float current_speed = Lerp(initial_speed, 0.0f, eased);
		velocity = move_direction * current_speed;

		// 到達 or 時間経過で停止
		if (t >= 1.0f || distance < 1.0f) {
			timer = 0.0f;
			velocity = Vector2D(0, 0);
			phase = Stop;
		}
		break;
	}

	case Stop: {
		if (timer >= stop_duration) {
			timer = 0.0f;
			phase = Homing;

			// ホーミング方向をロック
			Vector2D to_player = player->GetLocation() - this->location;
			homing_direction = to_player.Normalize();

			// 加速時間を距離に応じて決定（プレイヤーが近いと素早く加速）
			float distance = to_player.Length();
			const float min_duration = 0.2f;
			const float max_duration = 1.5f;
			const float max_distance = 300.0f;
			float ratio = Clamp(distance / max_distance, 0.0f, 1.0f);
			total_homing_duration = Lerp(min_duration, max_duration, 1.0f - ratio);
		}
		break;
	}

	case Homing: {
		float t = Clamp(timer / total_homing_duration, 0.0f, 1.0f);
		float eased = t * t; // easeInQuad
		float current_speed = Lerp(0.0f, homing_speed, eased);
		velocity = homing_direction * current_speed;

		if (t >= 1.0f) {
			phase = Straight;
		}
		break;
	}

	case Straight:
		// 直進中（velocityをそのまま使用）
		break;
	}

	location += velocity * delta_second;
}

void EnemyBullet6::Draw(const Vector2D& screen_offset) const
{
	DrawRotaGraph(location.x, location.y, 0.7f, 0.0f, image, TRUE);
}

void EnemyBullet6::Finalize()
{
}

void EnemyBullet6::OnHitCollision(GameObjectBase* hit_object)
{
}

void EnemyBullet6::SetPlayer(Player* p)
{
	player = p;
}

void EnemyBullet6::SetPhaseParams(float init_speed, float stop_dur, float homing_spd, float stop_x_random_offset, float stop_y_random_offset)
{
	initial_speed = init_speed;
	stop_duration = stop_dur;
	homing_speed = homing_spd;

	// 初期速度（真下）
	this->velocity = Vector2D(0, init_speed);

	// 停止目標座標（初期位置にランダムオフセットを加算）
	target_stop_position = this->location + Vector2D(stop_x_random_offset, stop_y_random_offset);
}