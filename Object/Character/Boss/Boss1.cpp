#include "Boss1.h"
#include "../../../Utility/EffectManager.h"
#include "../../../Utility/ShakeManager.h"
#include "../../../Utility/ScoreData.h"
#include "../../../Utility/SEManager.h"
#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../Character/Player/Player.h"
#include "../../Bullet/EnemyBullet/EnemyBullet3.h"

// ===== Boss1 難易度緩和用チューニング定数 =====
namespace {
    // 移動（HP減少時の“速くなる／振れ幅広がる”度合いを控えめに）
    constexpr float kMoveBaseSpeed = 0.90f;  // 元: 1.2f
    constexpr float kMoveSpeedScale = 0.30f;  // 元: 0.7f
    constexpr float kAmpXBase = 110.0f; // 元: 130.0f
    constexpr float kAmpXScale = 40.0f;  // 元: 70.0f
    constexpr float kAmpYBase = 20.0f;  // 元: 25.0f
    constexpr float kAmpYScale = 15.0f;  // 元: 25.0f

    // 攻撃間隔（HPが減っても撃つ間隔が短くなり過ぎないように）
    // 0.75～1.30秒の範囲で推移（元：0.50～1.10秒）
    constexpr float kShotIntervalMin = 0.55f;
    constexpr float kShotIntervalBonus = 0.4f; // 実際は kShotIntervalMin + kShotIntervalBonus * hp_ratio

    // 弾速の全体係数（全パターンを8割速に：視認性UP＆回避しやすく）
    constexpr float kBulletSpeedScale = 0.80f;

    // HPしきい値（“オーバードライブ”を弱める）
    constexpr float kPhaseMidRatio = 0.50f;
    constexpr float kPhaseLowRatio = 0.30f;
}


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
            hp -= 9.2f; // 減少量
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

    // ★HP割合：ハードコード(4000/8000)を廃止し、定義値に統一
    float hp_ratio = static_cast<float>(hp) / static_cast<float>(BOSS1_MAX_HP);
    if (hp_ratio < 0.0f) hp_ratio = 0.0f;
    if (hp_ratio > 1.0f) hp_ratio = 1.0f;

    // === 移動（難化を控えめに） ===
    const float amplitudeX = kAmpXBase + (1.0f - hp_ratio) * kAmpXScale;
    const float amplitudeY = kAmpYBase + (1.0f - hp_ratio) * kAmpYScale;
    const float speed = kMoveBaseSpeed + (1.0f - hp_ratio) * kMoveSpeedScale;

    location.x = D_WIN_MAX_X / 2 + sinf(move_timer * speed) * amplitudeX;
    location.y = target_pos.y + sinf(move_timer * speed * 2.0f) * amplitudeY;

    // === 攻撃間隔（短くなりすぎを抑制） ===
    shot_timer += delta_second;
    static float next_shot_interval = 0.6f;

    if (shot_timer > next_shot_interval)
    {
        static bool overdrive_triggered = false;

        if (hp_ratio < kPhaseLowRatio)
        {
            // ★“暴れ過ぎ”を抑える：演出シェイクのみ一度、攻撃はどちらか片方
            if (!overdrive_triggered) {
                overdrive_triggered = true;
                Singleton<ShakeManager>::GetInstance()->StartShake(0.4f, 14.0f, 14.0f);
            }

            // 交互にどちらか1回だけ（弾の総量を抑える）
            static bool toggle = false;
            if (toggle) ShotSpiral(delta_second);
            else        ShotAllRange();
            toggle = !toggle;
        }
        else if (hp_ratio < kPhaseMidRatio)
        {
            // 中間フェーズ：強攻撃の頻度を下げる
            if (GetRand(100) < 20) {
                // たまに強め（ただし FastSpiral は使わない）
                ShotAllRange();
            }
            else {
                // 通常ローテ（FastSpiral は除外）
                switch (attack_mode)
                {
                case 0: ShotSpiral(delta_second);   break;
                case 1: ShotAllRange();             break;
                case 2: ShotCrossShot();            break;
                case 3: ShotFanWide();              break;
                case 4: ShotWaveBullets();          break;
                case 5: ShotTripleSpread();         break;
                }
                attack_mode = (attack_mode + 1) % 6; // 0～5 の6種に
            }
        }
        else
        {
            // 高HP帯：通常ローテ（FastSpiral は封印して全体難度を下げる）
            switch (attack_mode)
            {
            case 0: ShotSpiral(delta_second);   break;
            case 1: ShotAllRange();             break;
            case 2: ShotCrossShot();            break;
            case 3: ShotFanWide();              break;
            case 4: ShotWaveBullets();          break;
            case 5: ShotTripleSpread();         break;
            }
            attack_mode = (attack_mode + 1) % 6;
        }

        // ★攻撃間隔：0.75～1.30秒に調整
        next_shot_interval = kShotIntervalMin + kShotIntervalBonus * hp_ratio;
        shot_timer = 0.0f;
    }
}


void Boss1::ShotSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f; // 回転角
    angle += 14.0f;            // 少しゆっくり（視認性↑）

    const int   strands = 2;                // リボン2本
    const int   taps = 7;                // 1本あたりの同時発射数（密度）
    const float step_deg = 360.0f / taps;    // 等間隔
    const float base_speed = 300.0f * kBulletSpeedScale;

    // ★一定間隔で“穴”を作る（tapsの中で2つ分は撃たない）
    const int hole_span = 2;   // 2連続で欠け
    static int hole_head = 0;  // 穴の開始インデックス（回転）
    hole_head = (hole_head + 1) % taps;

    auto skip_idx = [&](int idx) {
        for (int k = 0; k < hole_span; ++k) {
            if (((hole_head + k) % taps) == idx) return true;
        }
        return false;
        };

    for (int s = 0; s < strands; ++s)
    {
        float strand_phase = angle + (s * 180.0f / strands); // 互い違い

        for (int i = 0; i < taps; ++i)
        {
            if (skip_idx(i)) continue; // ★穴：リボンに通路を作る

            float deg = strand_phase + i * step_deg;
            float rad = deg * DX_PI / 180.0f;

            Vector2D vel(cosf(rad) * base_speed, sinf(rad) * base_speed);
            auto bullet = objm->CreateObject<EnemyBullet3>(location);
            bullet->SetVelocity(vel);
            bullet->SetPlayer(player);
        }
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}


void Boss1::ShotAllRange()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 少し弾数アップ（24 → 28）だが「穴」を作る
    const int    N = 28;
    const float  base_speed = 280.0f * kBulletSpeedScale;

    // ★回転するセーフレーン（2カ所・対向）を作る
    // gap_span：連続して空ける幅（指数ぶん）
    // gap_phase：ゆっくり回る“穴”の開始位置
    static float gap_phase = 0.0f;
    gap_phase += 0.35f;              // 穴の回転速度（ゆっくり）
    if (gap_phase >= 360.0f) gap_phase -= 360.0f;

    const int   gap_span = 3;      // 穴の太さ（指数3なら実角度は 3*(360/N)）
    int         gap_start0 = (int)(fmodf(gap_phase, 360.0f) / (360.0f / N));
    int         gap_start1 = (gap_start0 + N / 2) % N; // 反対側にも穴

    auto is_in_gap = [&](int idx, int start) {
        for (int k = 0; k < gap_span; ++k) {
            if (((start + k) % N) == idx) return true;
        }
        return false;
        };

    for (int i = 0; i < N; i++)
    {
        // 穴（どちらかに該当）なら“撃たない”
        if (is_in_gap(i, gap_start0) || is_in_gap(i, gap_start1)) continue;

        float rad = (i * (360.0f / N)) * DX_PI / 180.0f;
        Vector2D vel(cosf(rad) * base_speed, sinf(rad) * base_speed);

        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}


void Boss1::ShotCrossShot()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 4; i++)
    {
        float rad = (i * 90.0f) * DX_PI / 180.0f;
        Vector2D vel(cosf(rad) * (350.0f * kBulletSpeedScale),
            sinf(rad) * (350.0f * kBulletSpeedScale));
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotFanWide()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();

    // 前回より少し弾数アップ（16 → 20）だが、規則的に“欠け”を作る
    const int   bullet_count = 20;
    const float start_angle = -70.0f;   // 左端
    const float angle_step = 7.0f;     // 細かめ
    const float base_speed = 300.0f * kBulletSpeedScale;

    // ★格子感：周期的に弾を「抜く」
    // period=3, phase が回っていき、毎回“抜け位置”が少しずつズレる
    static int phase = 0;
    const int  period = 3;  // 3発ごとに1つ欠け
    phase = (phase + 1) % period;

    for (int i = 0; i < bullet_count; i++)
    {
        if ((i % period) == phase) continue; // 穴：格子のスキマ

        float deg = 90.0f + start_angle + i * angle_step;
        float rad = deg * DX_PI / 180.0f;

        Vector2D vel(cosf(rad) * base_speed, sinf(rad) * base_speed);
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}


void Boss1::ShotWaveBullets()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int i = 0; i < 5; i++)
    {
        float rad = (90.0f + ((i * 20.0f) - 40.0f)) * DX_PI / 180.0f;
        Vector2D vel(cosf(rad) * (250.0f * kBulletSpeedScale),
            sinf(rad) * (250.0f * kBulletSpeedScale));
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

void Boss1::ShotTripleSpread()
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    for (int dir = -1; dir <= 1; dir++)
    {
        Vector2D vel(dir * (120.0f * kBulletSpeedScale),
            (400.0f * kBulletSpeedScale)); // 左・中央・右
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    SEManager::GetInstance()->PlaySE(SE_NAME::EnemyShot);
}

// （※今回は “全体易化” 方針のため FastSpiral は使わない運用に変更）
// 使う必要があれば、弾数と回転速度を大幅抑制した下記で置き換え
void Boss1::ShotFastSpiral(float delta_second)
{
    auto objm = Singleton<GameObjectManager>::GetInstance();
    static float angle = 0.0f;

    const int N = 8; // 12 → 8（密度ダウン）
    for (int i = 0; i < N; i++)
    {
        float rad = (angle + i * (360.0f / N)) * DX_PI / 180.0f;
        Vector2D vel(cosf(rad) * (260.0f * kBulletSpeedScale),
            sinf(rad) * (260.0f * kBulletSpeedScale));
        auto bullet = objm->CreateObject<EnemyBullet3>(location);
        bullet->SetVelocity(vel);
        bullet->SetPlayer(player);
    }
    angle += 15.0f; // 25 → 15（回転をゆっくり）
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

        // （Draw 内）HP割合の算出を修正
        float hp_ratio = static_cast<float>(hp) / static_cast<float>(BOSS1_MAX_HP);
        if (hp_ratio < 0.0f) hp_ratio = 0.0f;
        if (hp_ratio > 1.0f) hp_ratio = 1.0f;


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
        if (player && player->GetNowType() == PlayerType::OmegaCode)
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
        DrawRotaGraph((int)location.x, (int)location.y, 3.5f, 0.0f, image, TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    // 最大HP = 8000
    float hp_ratio = (float)hp / BOSS1_MAX_HP;
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

        if (GetRand(70) == 1)
        {
            DropItems();
        }
    }

    if (type == eObjectType::eBeam)
    {
        beam_damage_timer += 1.0f / 60.0f;
        if (beam_damage_timer >= 0.15f)
        {
            hp -= 5;
            beam_damage_timer = 0.0f;
        }
    }
}
