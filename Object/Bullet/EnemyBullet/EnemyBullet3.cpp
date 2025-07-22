#include "EnemyBullet3.h"
#include "../../GameObjectManager.h"

EnemyBullet3::EnemyBullet3()
{
}

EnemyBullet3::~EnemyBullet3()
{
}

// 初期化処理
void EnemyBullet3::Initialize()
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
	//images = rm->GetImages("Resource/Image/Effect/anime_explosion03_01.png", 72, 12, 6, 112, 112);
	//image = images[36];

	bullet_1 = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime81.png", 6, 6, 1, 24, 24);
	bullet_2 = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime89.png", 4, 4, 1, 16, 32);

	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime81.png", 6, 6, 1, 24, 24));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/standard/anime81.png", 6, 6, 1, 24, 24));

	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime89.png", 4, 4, 1, 16, 32));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime89.png", 4, 4, 1, 16, 32));

	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime01.png", 8, 8, 1, 16, 16));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime01.png", 8, 8, 1, 16, 16));


	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime02.png", 6, 6, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime03.png", 8, 8, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime04.png", 7, 7, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime05.png", 7, 7, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime06.png", 6, 6, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime07.png", 6, 6, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime11.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime12.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime13.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime14.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime15.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime16.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime17.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime18.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime19.png", 4, 4, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime29.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime30.png", 6, 6, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime32.png", 6, 6, 1, 16, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime42.png", 5, 5, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime43.png", 6, 6, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime44.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime45.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime46.png", 4, 4, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime48.png", 5, 5, 1, 16, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime49.png", 5, 5, 1, 16, 16));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime50.png", 4, 4, 1, 16, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime51.png", 4, 4, 1, 16, 16));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime74.png", 4, 4, 1, 48, 48));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange_p/Standard/anime74.png", 4, 4, 1, 48, 48));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime74.png", 4, 4, 1, 48, 48));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Purple/standard/anime74.png", 4, 4, 1, 48, 48));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime75.png", 4, 4, 1, 32, 64));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime76.png", 4, 4, 1, 48, 96));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime77.png", 4, 4, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime78.png", 6, 6, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime79.png", 4, 4, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime80.png", 6, 6, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime81.png", 6, 6, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime82.png", 6, 6, 1, 24, 24));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime83.png", 6, 6, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime84.png", 6, 6, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime85.png", 6, 6, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime86.png", 6, 6, 1, 32, 32));
	//bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime87.png", 6, 6, 1, 32, 32));


	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime92.png", 4, 4, 1, 32, 32));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime94.png", 6, 6, 1, 32, 32));
	bullets.push_back(rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Orange/Standard/anime93.png", 6, 6, 1, 32, 32));

	//bullet[7] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime08.png", 7, 7, 1, 16, 24);
	//bullet[8] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime09.png", 8, 8, 1, 24, 24);
	//bullet[9] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime10.png", 2, 2, 1, 16, 16);
	//bullet[19] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime20.png", 4, 4, 1, 24, 24);
	//bullet[20] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime21.png", 3, 3, 1, 16, 16);
	//bullet[21] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime22.png", 8, 8, 1, 16, 16);
	//bullet[22] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime23.png", 6, 6, 1, 16, 16);
	//bullet[23] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime24.png", 6, 6, 1, 16, 16);
	//bullet[24] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime25.png", 6, 6, 1, 16, 16);
	//bullet[25] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime26.png", 4, 4, 1, 16, 16);
	//bullet[26] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime27.png", 6, 6, 1, 16, 16);
	//bullet[27] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime28.png", 7, 7, 1, 16, 16);
	//bullet[30] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime31.png", 6, 6, 1, 32, 32);
	//bullet[32] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime33.png", 6, 6, 1, 16, 32);
	//bullet[33] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime34.png", 4, 4, 1, 16, 32);
	//bullet[34] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime35.png", 4, 4, 1, 8, 24);
	//bullet[35] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime36.png", 4, 4, 1, 8, 56);
	//bullet[36] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime37.png", 4, 4, 1, 24, 24);
	//bullet[37] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime38.png", 6, 6, 1, 24, 24);
	//bullet[38] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime39.png", 4, 4, 1, 32, 16);
	//bullet[39] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime40.png", 3, 3, 1, 32, 16);
	//bullet[40] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime41.png", 4, 4, 1, 32, 32);
	//bullet[46] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime47.png", 4, 4, 1, 16, 64);
	//bullet[51] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime52.png", 4, 4, 1, 16, 32);
	//bullet[52] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime53.png", 4, 4, 1, 16, 32);
	//bullet[53] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime54.png", 4, 4, 1, 16, 32);
	//bullet[54] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime55.png", 4, 4, 1, 16, 32);
	//bullet[55] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime56.png", 4, 4, 1, 16, 32);
	//bullet[56] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime57.png", 4, 4, 1, 16, 32);
	//bullet[57] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime58.png", 4, 4, 1, 16, 32);
	//bullet[58] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime59.png", 4, 4, 1, 16, 32);
	//bullet[59] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime60.png", 4, 4, 1, 16, 32);
	//bullet[60] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime61.png", 4, 4, 1, 16, 32);
	//bullet[61] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime62.png", 4, 4, 1, 16, 32);
	//bullet[62] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime63.png", 4, 4, 1, 16, 32);
	//bullet[63] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime64.png", 4, 4, 1, 16, 32);
	//bullet[64] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime65.png", 4, 4, 1, 16, 32);
	//bullet[65] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime66.png", 4, 4, 1, 16, 32);
	//bullet[66] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime67.png", 4, 4, 1, 16, 32);
	//bullet[67] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime68.png", 4, 4, 1, 16, 32);
	//bullet[68] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime69.png", 4, 4, 1, 16, 32);
	//bullet[69] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime70.png", 4, 4, 1, 16, 32);
	//bullet[70] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime71.png", 4, 4, 1, 16, 32);
	//bullet[71] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime72.png", 4, 4, 1, 16, 32);
	//bullet[72] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime73.png", 4, 4, 1, 16, 32);
	//bullet[87] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime88.png", 4, 4, 1, 16, 32);
	//bullet[88] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime89.png", 4, 4, 1, 16, 32);
	//bullet[89] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime90.png", 4, 4, 1, 16, 32);
	//bullet[90] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime91.png", 4, 4, 1, 16, 32);
	//bullet[94] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime95.png", 4, 4, 1, 16, 32);
	//bullet[95] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime96.png", 4, 4, 1, 16, 32);
	//bullet[96] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime97.png", 4, 4, 1, 16, 32);
	//bullet[97] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime98.png", 4, 4, 1, 16, 32);
	//bullet[98] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime99.png", 4, 4, 1, 16, 32);
	//bullet[99] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime100.png", 4, 4, 1, 16, 32);
	//bullet[100] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime101.png", 4, 4, 1, 16, 32);
	//bullet[101] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime102.png", 4, 4, 1, 16, 32);
	//bullet[102] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime103.png", 4, 4, 1, 16, 32);
	//bullet[103] = rm->GetImages("Resource/Image/Object/Enemy/Enemy_Bullet/Blue/standard/anime104.png", 4, 4, 1, 16, 32);

	bullet = { bullets[6][0], bullets[7][0],  bullets[6][1],  bullets[7][1],  bullets[6][2],  bullets[7][2],  bullets[6][3],  bullets[7][3], };
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="delata_second">1フレーム当たりの時間</param>
void EnemyBullet3::Update(float delta_second)
{
	//GameObjectBase::AnimationControl(delta_second, images, {36, 37, 38, 39, 40, 41, 42, 43, 42, 41, 40, 39, 38, 37}, 10.0f);
	//GameObjectBase::AnimationControl(delta_second, images, {0, 1, 2, 3, 2, 1}, 10.0f);
	move_time += delta_second;

	// 弾が画面外に出たら削除（移動方向によって判定を変える）
	if (location.y < -100.0f || // 上方向
		location.y > D_WIN_MAX_Y + 100.0f ||
		location.x > D_WIN_MAX_X - 100.0f ||
		location.x < 0 + 100.0f) // 下方向
	{
		is_destroy = true;
	}


	std::vector<int> animation_num = { 0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1 };
	//フレームレートで時間を計測
	animation_time += delta_second;
	//8秒経ったら画像を切り替える
	if (animation_time >= 0.05f)
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
		image = bullet[animation_num[animation_count]];
	}


	location += velocity * delta_second;
}

/// <summary>
/// 描画処理
/// </summary>
/// <param name="screen_offset"></param>
void EnemyBullet3::Draw(const Vector2D& screen_offset) const
{
	float angle = move_time * 10.0f; // 回転速度
	float hw = box_size.x;
	float hh = box_size.y;

	float cosA = cos(angle);
	float sinA = sin(angle);

	Vector2D p1(-hw, -hh);
	Vector2D p2(hw, -hh);
	Vector2D p3(hw, hh);
	Vector2D p4(-hw, hh);

	auto rotate = [&](Vector2D p) {
		return Vector2D(
			location.x + p.x * cosA - p.y * sinA,
			location.y + p.x * sinA + p.y * cosA
		);
		};

	p1 = rotate(p1);
	p2 = rotate(p2);
	p3 = rotate(p3);
	p4 = rotate(p4);

	int color = GetColor(255, 255, 0);
	//DrawTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, color, TRUE);
	//DrawTriangle(p1.x, p1.y, p3.x, p3.y, p4.x, p4.y, color, TRUE);

	DrawRotaGraph(location.x, location.y, 0.7f, angle, image, TRUE);
}

// 終了時処理
void EnemyBullet3::Finalize()
{
}

void EnemyBullet3::OnHitCollision(GameObjectBase* hit_object)
{
	if (hit_object->GetCollision().object_type == eObjectType::ePlayer ||
		hit_object->GetCollision().object_type == eObjectType::eDefenceShot)
	{
		is_destroy = true;
	}
}

void EnemyBullet3::SetAttackPattrn(int pattrn)
{
	attack_pattrn = pattrn;
	if (pattrn <= 1)
	{
		bullet = { bullets[8][0], bullets[9][0],  bullets[8][1],  bullets[9][1],  bullets[8][2],  bullets[9][2],  bullets[8][3],  bullets[9][3], };

		//bullet.push_back(bullets[0][0]);
		//bullet.push_back(bullets[1][0]);
	}
	else if (pattrn == 2)
	{
		bullet = { bullets[6][0], bullets[7][0],  bullets[6][1],  bullets[7][1],  bullets[6][2],  bullets[7][2],  bullets[6][3],  bullets[7][3], };

		//bullet.push_back(bullets[2][0]);
		//bullet.push_back(bullets[3][0]);
	}
	else
	{
		bullet = { bullets[6][0], bullets[7][0],  bullets[6][1],  bullets[7][1],  bullets[6][2],  bullets[7][2],  bullets[6][3],  bullets[7][3], };

	}

}
