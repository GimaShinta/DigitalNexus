#include "Boss1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ShakeManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../Character/Player/Player.h"
#include "../../Bullet/EnemyBullet/EnemyBullet3.h"

Boss1::Boss1() {}
Boss1::~Boss1() {}

void Boss1::Initialize()
{

    // 衝突設定
    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemy;
    collision.hit_object_type.push_back(eObjectType::eAttackShot);
    collision.hit_object_type.push_back(eObjectType::eBeam);

    box_size = Vector2D(70, 50);
    //hp = 8000;
    hp = BOSS1_MAX_HP;  // 8000に変更

    is_alive = true;
    pattern = BossPattern::Entrance;

    location = Vector2D(D_WIN_MAX_X / 2, -250);
    target_pos = Vector2D(D_WIN_MAX_X / 2, 180);
    velocity = Vector2D(0, 0);

    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    //images = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy75_b.png", 6, 6, 1, 40, 40);
    images = rm->GetImages("Resource/Image/Object/Enemy/Zako3/anime_enemy87_a.png", 6, 6, 1, 56, 56);
    image = images[0];

    sound_destroy = rm->GetSounds("Resource/sound/se/se_effect/kill_4.mp3");
    ChangeVolumeSoundMem(255, sound_destroy);

    alpha = 0.0f;
    pattern_timer = 0.0f;




}

void Boss1::Update(float delta_second)
{
    pattern_timer += delta_second;

    // 撃破判定
    if (hp <= 0 && pattern != BossPattern::Dead)
    {
        pattern = BossPattern::Dead;
        is_alive = false;
        explosion_timer = 0.0f;
        explosions_started = false;
        explosion_count = 0;
        final_explosion_done = false;

        PlaySoundMem(sound_destroy, DX_PLAYTYPE_BACK);
        Singleton<ShakeManager>::GetInstance()->StartShake(1.0f, 30.0f, 30.0f);
    }

    // 状態ごとの挙動
    switch (pattern)
    {
    case BossPattern::Entrance:
        UpdateEntrance(delta_second);
        break;

    case BossPattern::Hovering:
        UpdateHovering(delta_second);
        break;

    case BossPattern::Dead:
        ExplosionEffect(delta_second);
        break;
    }
    if (pattern != BossPattern::Dead)
    {
        // 波紋・オーラ用のタイマーを更新
        aura_timer += delta_second;
        ripple_timer += delta_second;
    }

    // Dead以外はアニメーション更新
    if (pattern != BossPattern::Dead)
    {
        int frame = static_cast<int>((GetNowCount() / 10) % images.size());
        image = images[frame];
    }
    // 戦闘開始後は時間経過でHPを減らす
    if (battle_started && pattern != BossPattern::Dead)
    {
        damage_timer += delta_second;
        if (damage_timer >= 0.05f) // 0.05秒ごとに減少
        {
            damage_timer = 0.0f;
            hp -= 0.2; // 減少量
        }
    }

    on_hit = false;

}

void Boss1::UpdateEntrance(float delta_second)
{
    // y方向をイージングで移動
    float speed = 3.0f; // 大きいほど早く到達
    location.y += (target_pos.y - location.y) * speed * delta_second;

    if (fabs(location.y - target_pos.y) < 1.0f)
    {
        location.y = target_pos.y;
        pattern = BossPattern::Hovering;
        shot_timer = 0.0f;
        move_timer = 0.0f;
        battle_started = true; // ★戦闘開始
    }


    // フェードイン
    alpha += delta_second * 150.0f;
    if (alpha > 255.0f) alpha = 255.0f;
}

void Boss1::UpdateHovering(float delta_second)
{
    move_timer += delta_second;

    // HP割合を算出
    float hp_ratio = (float)hp / 4000.0f;
    if (hp_ratio < 0.0f) hp_ratio = 0.0f;

    // === 移動速度・振れ幅をHPに応じて変化 ===
    float amplitudeX = 130.0f + (1.0f - hp_ratio) * 70.0f; // HP減少で振れ幅拡大
    float amplitudeY = 25.0f + (1.0f - hp_ratio) * 25.0f;  // 縦揺れ増加
    float speed = 1.2f + (1.0f - hp_ratio) * 0.7f;         // HP減少で速くなる

    location.x = D_WIN_MAX_X / 2 + sin(move_timer * speed) * amplitudeX;
    location.y = target_pos.y + sin(move_timer * speed * 2.0f) * amplitudeY;

    // === 攻撃間隔も変化 ===
    shot_timer += delta_second;
    static float next_shot_interval = 0.8f;

    if (shot_timer > next_shot_interval)
    {
        static bool overdrive_triggered = false;

        if (hp_ratio < 0.3f)
        {
            if (!overdrive_triggered)
            {
                overdrive_triggered = true;
                Singleton<ShakeManager>::GetInstance()->StartShake(0.6f, 20.0f, 20.0f);
            }

            // オーバードライブ攻撃（ここから頻繁に撃つように）
            ShotFastSpiral(delta_second);
            ShotAllRange();
        }
        else if (hp_ratio < 0.5f)
        {
            // 通常とランダムで強攻撃
            if (GetRand(100) < 30)
            {
                ShotAllRange();
                ShotFastSpiral(delta_second);
            }
            else
            {
                // 通常攻撃ローテーション
                switch (attack_mode)
                {
                case 0: ShotSpiral(delta_second); break;
                case 1: ShotAllRange(); break;
                case 2: ShotCrossShot(); break;
                case 3: ShotFanWide(); break;
                case 4: ShotWaveBullets(); break;
                case 5: ShotTripleSpread(); break;
                case 6: ShotFastSpiral(delta_second); break;
                }
                attack_mode = (attack_mode + 1) % 7;
            }
        }
        else
        {
            // 通常攻撃
            switch (attack_mode)
            {
            case 0: ShotSpiral(delta_second); break;
            case 1: ShotAllRange(); break;
            case 2: ShotCrossShot(); break;
            case 3: ShotFanWide(); break;
            case 4: ShotWaveBullets(); break;
            case 5: ShotTripleSpread(); break;
            case 6: ShotFastSpiral(delta_second); break;
            }
            attack_mode = (attack_mode + 1) % 7;
        }


        // HPに応じて攻撃間隔を短く
        next_shot_interval = 0.6f * hp_ratio + 0.5f;
        shot_timer = 0.0f;
    }


}




void Boss1::ShotSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f;

    for (int dir = -1; dir <= 1; dir += 2)
    {
        for (int i = 0; i < 6; i++)
        {
            float rad = (angle + dir * i * 60.0f) * DX_PI / 180.0f;
            Vector2D vel(cos(rad) * 300.0f, sin(rad) * 300.0f);
            auto bullet = objm->CreateObject<EnemyBullet3>(location);
            bullet->SetVelocity(vel);
        }
    }
    angle += 15.0f;
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotAllRange()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 24; i++)
    {
        float rad = (i * 15.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 280.0f, sin(rad) * 280.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotCrossShot()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 4; i++)
    {
        float rad = (i * 90.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 350.0f, sin(rad) * 350.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotFanWide()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 扇状に下方向へばらまく
    const int bullet_count = 24;
    const float start_angle = -60.0f;   // 左下側の角度
    const float angle_step = 5.0f;      // 角度の広がり幅

    for (int i = 0; i < bullet_count; i++)
    {
        // 下方向を基準に扇状に広げる
        float rad = (90.0f + start_angle + i * angle_step) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 300.0f, sin(rad) * 300.0f);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }

    // 撃つたびに派手なSE
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}



void Boss1::ShotWaveBullets()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 下方向を基準に5方向へ撃つ
    for (int i = 0; i < 5; i++)
    {
        float rad = (90.0f + ((i * 20.0f) - 40.0f)) * DX_PI / 180.0f; // 下方向基準
        Vector2D vel(cos(rad) * 250.0f, sin(rad) * 250.0f);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }

    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}



void Boss1::ShotTripleSpread()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int dir = -1; dir <= 1; dir++)
    {
        Vector2D vel(dir * 120.0f, 400.0f); // 左・中央・右
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotFastSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f;

    for (int i = 0; i < 12; i++)
    {
        float rad = (angle + i * 30.0f) * DX_PI / 180.0f;
        Vector2D vel(cos(rad) * 380.0f, sin(rad) * 380.0f);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
    }
    angle += 25.0f; // 高速回転
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ExplosionEffect(float delta_second)
{
    explosion_timer += delta_second;

    if (!explosions_started)
    {
        Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.06f, false);
        explosions_started = true;
    }
    else if (explosion_timer > 0.1f && explosion_count <= 10)
    {
        // 複数箇所同時爆発
        for (int i = 0; i < 3; i++)
        {
            Vector2D randPos = location + Vector2D(GetRand(180) - 90, GetRand(180) - 90);
            Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, randPos, 0.05f, false);
        }
        explosion_count++;
        explosion_timer = 0.0f;
    }

    if (explosion_count > 10)
    {
        if (!final_explosion_done)
        {
            Singleton<EffectManager>::GetInstance()->PlayerAnimation(EffectName::eExprotion2, location, 0.08f, false);
            final_explosion_done = true;
            explosion_timer = 0.0f;
        }
        else if (explosion_timer > 0.2f)
        {
            Singleton<ScoreData>::GetInstance()->AddScore(15000);
            is_destroy = true;
        }
    }
}

void Boss1::Draw(const Vector2D& screen_offset) const
{
    // 本体画像の描画は撃破後に消すが、HPゲージは描画させたいのでこの位置に分岐
    bool draw_body = !(pattern == BossPattern::Dead && final_explosion_done);


    if (draw_body)
    {

        // --- HP割合を計算 ---
        float hp_ratio = (float)hp / 8000.0f;  // 最大HP4000として割合を求める
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;

        // --- 色変化（HPが減るほど赤寄りになる） ---
        // 緑成分をHP割合で減らす → HP満タン: 緑が多い, HP減少: 赤が強くなる
        int r = (int)(120 + (1.0f - hp_ratio) * 135);   // 120～255
        int g = (int)(180 * hp_ratio);                  // 180～0
        int b = (int)(100 * hp_ratio + 30);             // 130～30（暗めになる）

        // --- 波紋（二重） ---
        float ripple_radius = 65.0f + sinf(ripple_timer * 4.0f) * 15.0f;
        int ripple_alpha = 70 + (int)(sinf(ripple_timer * 10.0f) * 30.0f + 30.0f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, ripple_alpha);
        for (int i = 0; i < 2; i++)
        {
            float distortion = (i == 0) ? 1.0f : 1.2f;
            int thickness = (i == 0) ? 3 : 2;
            DrawCircle((int)location.x + (i * 2), (int)location.y + (i * 2),
                (int)(ripple_radius * distortion),
                GetColor(r, g, b), FALSE, thickness);
        }

        // --- オーラ ---
        int aura_alpha = 50 + (int)(sinf(aura_timer * 8.0f) * 20.0f + 20.0f);
        SetDrawBlendMode(DX_BLENDMODE_ADD, aura_alpha);
        int aura_size_x = 100 + (int)(sinf(aura_timer * 6.0f) * 20.0f);
        int aura_size_y = 85 + (int)(cosf(aura_timer * 7.0f) * 15.0f);
        DrawOval((int)location.x, (int)location.y,
            aura_size_x, aura_size_y, GetColor(r, g, b), TRUE);

        // --- ボス本体 ---
        int draw_alpha = (int)alpha;
        if (draw_alpha < 180) draw_alpha = 180;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, draw_alpha);
        DrawRotaGraph((int)location.x, (int)location.y, 3.5f, 0.0f, image, TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    // 最大HP = 8000
    float hp_ratio = (float)hp / 8000.0f;
    if (hp_ratio < 0.0f) hp_ratio = 0.0f;
    if (hp_ratio > 1.0f) hp_ratio = 1.0f;

    // フェードイン（2秒）
    float fade_ratio = (pattern_timer < 0.0f) ? 0.0f : ((pattern_timer > 2.0f) ? 1.0f : (pattern_timer / 2.0f));
    int bar_alpha = (int)(fade_ratio * 200); // 最大200に増加（やや明るく）

    // ★ ゲージ幅：画面いっぱい（ほぼ）640px
    const int bar_width = 640;
    const int bar_height = 6;
    const int bar_x = (D_WIN_MAX_X - bar_width) / 2;
    const int bar_y = 20;

    // ゆらぎ
    float wave_offset = sinf(GetNowCount() * 0.01f) * 1.0f;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, bar_alpha);

    // 背景（明るめグレー）
    DrawBox(bar_x, bar_y + wave_offset, bar_x + bar_width, bar_y + bar_height + wave_offset, GetColor(50, 50, 50), TRUE);

    // 本体バー（明るい赤：白っぽい）
    int bar_color = GetColor(240, 80, 80);
    DrawBox(bar_x, bar_y + wave_offset, bar_x + (int)(bar_width * hp_ratio), bar_y + bar_height + wave_offset, bar_color, TRUE);

    // 枠線（明るめの白）
    DrawBox(bar_x, bar_y + wave_offset, bar_x + bar_width, bar_y + bar_height + wave_offset, GetColor(180, 180, 180), FALSE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);



}

void Boss1::Finalize() {}

void Boss1::SetPattern(BossPattern new_pattern)
{
    pattern = new_pattern;
    pattern_timer = 0.0f;
}

bool Boss1::GetIsAlive() const
{
    return is_alive;
}

void Boss1::OnHitCollision(GameObjectBase* hit_object)
{
    auto type = hit_object->GetCollision().object_type;

    if (type == eObjectType::eAttackShot)
    {
        hp -= 10;
    }

    if (type == eObjectType::eBeam)
    {
        beam_damage_timer += 1.0f / 60.0f;
        if (beam_damage_timer >= 0.35f)
        {
            hp -= 10;
            beam_damage_timer = 0.0f;
        }
    }
}
