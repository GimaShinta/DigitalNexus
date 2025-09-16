#include "Boss2RotatingPart.h"
#include "Boss2.h"

#include "../../../Utility/ResourceManager.h"
#include "../../../Object/GameObjectManager.h"
#include "../../Bullet/EnemyBullet/EnemyBullet1.h"
#include "../../Bullet/EnemyBullet/EnemyBullet2.h"
#include "../../../Object/Character/Player/Player.h" // ADJUST: 実際のパスに合わせて

#include <cmath>

namespace
{
    inline float DegToRad(float d)
    {
        return d * 3.1415926535f / 180.0f;
    }
    inline float TwoPi()
    {
        return 6.283185307f;
    }

    static float AngleOf(const Vector2D& v)
    {
        return std::atan2(v.y, v.x);
    }

    // [-pi,pi]へ正規化
    static float WrapPi(float a)
    {
        while (a > 3.14159265f) a -= TwoPi();
        while (a < -3.14159265f) a += TwoPi();
        return a;
    }
}

Boss2RotatingPart::Boss2RotatingPart()
{}

void Boss2RotatingPart::Initialize()
{
    // 表示と当たり
    z_layer = 3;
    box_size = Vector2D(20.0f, 20.0f);

    collision.is_blocking = true;
    collision.object_type = eObjectType::eEnemyPart;
    collision.hit_object_type.clear();
    collision.hit_object_type.push_back(eObjectType::eAttackShot);

    // 見た目
    ResourceManager* rm = Singleton<ResourceManager>::GetInstance();
    auto imgs = rm->GetImages("Resource/Image/Object/Enemy/Boss/s1_Boss/anime_enemy74_a.png", 6, 6, 1, 48, 48);
    image = imgs.empty() ? -1 : imgs[0];

    // タイマ類
    shot_timer = 0.0f;
    mode_timer = 0.0f;
    spiral_phase = 0.0f;
    pendulum_time = 0.0f;

    // “半追尾”角度初期化
    sh_target_angle_rad = 0.0f;
    sh_current_angle_rad = 0.0f;

    pattern = BulletPattern::Spiral;
}

void Boss2RotatingPart::SetUp(Boss2* boss_, float radius_, float angle_offset_)
{
    boss = boss_;
    radius = radius_;
    angle_offset = angle_offset_; // 未使用（互換）
}

float Boss2RotatingPart::GetAngleGlobal() const
{
    const float slot_deg = (ring_total > 0) ? (360.0f / ring_total) * ring_index : 0.0f;
    const float deg = (boss ? boss->GetOrbitBaseAngle() : 0.0f) + slot_deg;
    float m = std::fmod(deg, 360.0f);
    if (m < 0.0f) m += 360.0f;
    return m;
}

void Boss2RotatingPart::Update(float delta_second)
{
    if (!boss)
    {
        __super::Update(delta_second); return;
    }

    // 円周上へ等間隔配置
    const float deg = GetAngleGlobal();
    const float rad = DegToRad(deg);
    const Vector2D center = boss->GetLocation();
    location.x = center.x + std::cos(rad) * radius;
    location.y = center.y + std::sin(rad) * radius;

    // イントロ中は撃たない
    if (boss->IsIntroActive())
    {
        __super::Update(delta_second); return;
    }

    // HP連動パターン（有効時）
    if (hp_phase_enable) UpdateHpDrivenPattern();

    // Boss2RotatingPart.cpp Update内
    rotation_angle += delta_second * 2.0f; // 自己回転


    // 自動サイクル
    mode_timer += delta_second;
    if (mode_timer >= pattern_cycle)
    {
        mode_timer = 0.0f; NextPattern();
    }

    // ===== 弾幕 =====
    if (is_bullet_part)
    {
        shot_timer += delta_second;

        switch (pattern)
        {
            case BulletPattern::Spiral:
            {
                if (shot_timer >= interval_spiral)
                {
                    shot_timer = 0.0f;

                    float a = rad + spiral_phase;
                    FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_spiral);
                    float delta = DegToRad(12.0f) * ((ring_index % 2 == 0) ? 1.0f : -1.0f);
                    spiral_phase += delta;
                    if (spiral_phase > TwoPi()) spiral_phase -= TwoPi();
                    if (spiral_phase < -TwoPi()) spiral_phase += TwoPi();
                }
                break;
            }
            case BulletPattern::NWay:
            {
                if (shot_timer >= interval_nway)
                {
                    shot_timer = 0.0f;
                    int m = nway_count;
                    float spread = DegToRad(nway_spread_deg);
                    float start = (m > 1 ? -spread * 0.5f : 0.0f);
                    float step = (m > 1 ? spread / (m - 1) : 0.0f);
                    for (int i = 0; i < m; ++i)
                        FireBullet(Vector2D(std::cos(rad + start + step * i), std::sin(rad + start + step * i)),
                                   speed_nway);
                }
                break;
            }
            case BulletPattern::Ring:
            {
                if (shot_timer >= interval_ring)
                {
                    shot_timer = 0.0f;
                    int cnt = (ring_count < 3) ? 3 : ring_count;
                    for (int i = 0; i < cnt; ++i)
                    {
                        float a = (TwoPi() / cnt) * i;
                        FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_ring);
                    }
                }
                break;
            }
            case BulletPattern::Pendulum:
            {
                pendulum_time += delta_second;
                if (shot_timer >= interval_pendulum)
                {
                    shot_timer = 0.0f;
                    float t = (pendulum_period_sec <= 0.0f) ? 0.0f : std::fmod(pendulum_time / pendulum_period_sec, 1.0f);
                    float sweep = std::sinf(t * TwoPi()) * DegToRad(pendulum_amplitude_deg);
                    int   m = (pendulum_way < 1) ? 1 : pendulum_way;
                    float spread = DegToRad(nway_spread_deg);
                    float start = (m > 1 ? -spread * 0.5f : 0.0f);
                    float step = (m > 1 ? spread / (m - 1) : 0.0f);
                    for (int i = 0; i < m; ++i)
                    {
                        float a = (rad + sweep) + start + step * i;
                        FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_pendulum);
                    }
                }
                break;
            }
            case BulletPattern::Burst:
            {
                if (burst_left <= 0)
                {
                    if (shot_timer >= interval_burst_cool)
                    {
                        shot_timer = 0.0f;
                        burst_left = (burst_waves < 1) ? 1 : burst_waves;
                        burst_inner_timer = 0.0f;
                    }
                }
                else
                {
                    burst_inner_timer += delta_second;
                    if (burst_inner_timer >= burst_inner_interval)
                    {
                        burst_inner_timer = 0.0f;
                        int cnt = (burst_way < 3) ? 3 : burst_way;
                        float offset = (float)(ring_index % 2) * (TwoPi() / (cnt * 2));
                        for (int i = 0; i < cnt; ++i)
                        {
                            float a = offset + (TwoPi() / cnt) * i;
                            FireBullet(Vector2D(std::cos(a), std::sin(a)), speed_burst);
                        }
                        --burst_left;
                    }
                }
                break;
            }
            case BulletPattern::SemiHoming:
            {
                if (shot_timer >= interval_semihoming)
                {
                    shot_timer = 0.0f;

                    // 目標角（プレイヤー方向）
                    Vector2D tgt;
                    if (TryGetPlayerPos(tgt))
                    {
                        Vector2D to_p = tgt - location;
                        sh_target_angle_rad = AngleOf(to_p);
                    }
                    else
                    {
                        // プレイヤー不明なら外向き
                        sh_target_angle_rad = rad;
                    }

                    // “半追尾”＝角度をローパスで少しずつ近づける
                    float diff = WrapPi(sh_target_angle_rad - sh_current_angle_rad);
                    sh_current_angle_rad += diff * sh_follow_k; // 位相遅延

                    FireBullet(Vector2D(std::cos(sh_current_angle_rad), std::sin(sh_current_angle_rad)),
                               speed_semihoming);
                }
                break;
            }
            case BulletPattern::DecelBurst:
            {
                if (shot_timer >= interval_decelburst)
                {
                    shot_timer = 0.0f;

                    // 「減速→停止→放射」をする弾を1発発射
                    // 実装は “特殊弾なし” でも動くよう、ここでタスクを1行で作れる弾を使う
                    // ⇒ CreateObject<EnemyBulletDecelBurst>
                    auto* gom = Singleton<GameObjectManager>::GetInstance();
                    auto* b = gom->CreateObject<EnemyBullet1>(location); // 既定弾を仮に生成
                    b->SetPlayer(player);
                    if (b)
                    {
                        // ADJUST HERE: もし「特殊弾クラス（EnemyBulletDecelBurst）」を用意できるなら
                        // そちらを生成して、初速/db_decel_per_sec/db_min_speed/db_ring_way/db_ring_speed を渡してください。
                        // 既定弾しか無い場合は“見た目近似”として少し遅めの弾を出す＋将来の放射をここで予約発射します。
                    }

                    // 近似版：停止地点を概算し、そこでリングをスポーン（演出は近似だが見た目の気持ちは出る）
                    // v0, a<0 で停止までの時間 t_stop = (v0 - v_min)/|a|
                   // 旧:
// float t_stop = (db_initial_speed - db_min_speed) / std::max(1.0f, db_decel_per_sec);

// 新:
                    const float denom = (db_decel_per_sec < 1.0f) ? 1.0f : db_decel_per_sec;
                    const float t_stop = (db_initial_speed - db_min_speed) / denom;
                    float a = rad; // 外向き
                    Vector2D dir(std::cos(a), std::sin(a));
                    Vector2D stop_pos = location + dir * ((db_initial_speed + db_min_speed) * 0.5f * t_stop);

                    // “将来のリング発射”を予約（簡易：今すぐ発射に近いが、演出の肝はリングなのでOK）
                    int cnt = (db_ring_way < 3) ? 3 : db_ring_way;
                    for (int i = 0; i < cnt; ++i)
                    {
                        float aa = (TwoPi() / cnt) * i;
                        Vector2D d(std::cos(aa), std::sin(aa));
                        // 予約の代わりに即時：stop_pos から発射
                        auto* bb = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(stop_pos);
                        bb->SetPlayer(player);
                        if (bb)
                        {
                            constexpr float kBase = 200.0f;
                            Vector2D n = SafeNormalize(d);
                            bb->SetVelocity(n * (db_ring_speed / kBase));
                        }
                    }
                }
                break;
            }
            case BulletPattern::TimedSplit:
            {
                if (shot_timer >= interval_tsplit)
                {
                    shot_timer = 0.0f;

                    // “時限分裂”＝一定時間後にリングへ
                    // 特殊弾クラスが作れるなら EnemyBulletTimedSplit を生成し、ts_split_time 等を渡す
                    // ここでは近似として、分裂予定点を概算し、そこでリングを即時発射
                    float a = rad; // 外向き
                    Vector2D dir(std::cos(a), std::sin(a));
                    Vector2D split_pos = location + dir * (ts_initial_speed * ts_split_time);

                    int cnt = (ts_ring_way < 3) ? 3 : ts_ring_way;
                    for (int i = 0; i < cnt; ++i)
                    {
                        float aa = (TwoPi() / cnt) * i;
                        Vector2D d(std::cos(aa), std::sin(aa));
                        auto* bb = Singleton<GameObjectManager>::GetInstance()->CreateObject<EnemyBullet1>(split_pos);
                        bb->SetPlayer(player);
                        if (bb)
                        {
                            constexpr float kBase = 200.0f;
                            Vector2D n = SafeNormalize(d);
                            bb->SetVelocity(n * (ts_ring_speed / kBase));
                        }
                    }
                }
                break;
            }
        } // switch
    }

    __super::Update(delta_second);
}

void Boss2RotatingPart::Draw(const Vector2D& /*screen_offset*/) const
{
    if (image < 0) return;
    float rot = DegToRad(GetAngleGlobal());
    // Boss2RotatingPart.cpp Draw
    if (player && player->GetNowType() == PlayerType::OmegaCode)
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    if (image >= 0) {
        DrawRotaGraph((int)location.x, (int)location.y, 1.5f, rotation_angle, image, TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

}

// ===== Utility =====

Vector2D Boss2RotatingPart::SafeNormalize(const Vector2D& v)
{
    float len2 = v.x * v.x + v.y * v.y;
    if (len2 <= 0.000001f) return Vector2D(1.0f, 0.0f);
    float inv = 1.0f / std::sqrt(len2);
    return Vector2D(v.x * inv, v.y * inv);
}

void Boss2RotatingPart::FireBullet(const Vector2D& dir, float speed)
{
    auto* objm = Singleton<GameObjectManager>::GetInstance();
    auto* b = objm->CreateObject<EnemyBullet1>(location);
    b->SetPlayer(player);
    if (!b) return;

    // EnemyBullet1 は Update で (velocity * 200) を使う前提のためスケール調整
    constexpr float kBase = 200.0f;
    Vector2D n = SafeNormalize(dir);
    b->SetVelocity(n * (speed / kBase));
}

void Boss2RotatingPart::ResetPatternState()
{
    shot_timer = 0.0f;
    mode_timer = 0.0f;
    spiral_phase = 0.0f;
    pendulum_time = 0.0f;
    burst_left = 0;
    burst_inner_timer = 0.0f;

    // 半追尾の現在角は外向きに初期化
    sh_current_angle_rad = DegToRad(GetAngleGlobal());
    sh_target_angle_rad = sh_current_angle_rad;
}

void Boss2RotatingPart::NextPattern()
{
    switch (pattern)
    {
        case BulletPattern::Spiral:     pattern = BulletPattern::NWay;       break;
        case BulletPattern::NWay:       pattern = BulletPattern::Ring;       break;
        case BulletPattern::Ring:       pattern = BulletPattern::Spiral;     break;
      //case BulletPattern::Pendulum:   pattern = BulletPattern::Burst;      break;
      //case BulletPattern::Burst:      pattern = BulletPattern::SemiHoming; break;
      //case BulletPattern::SemiHoming: pattern = BulletPattern::DecelBurst; break;
      //case BulletPattern::DecelBurst: pattern = BulletPattern::TimedSplit; break;
      //case BulletPattern::TimedSplit: pattern = BulletPattern::Spiral;     break;
        default:   pattern = BulletPattern::Spiral;                          break;

    }
    ResetPatternState();
}

// ===== HP連動 =====

float Boss2RotatingPart::GetBossHpRate() const
{
    // ADJUST HERE: Boss2 の HP 取得APIに合わせて1行で返す
    // 例1) return boss->GetHp() / std::max(1.0f, boss->GetHpMax());
    // 例2) return boss->GetHpRate(); // 0.0～1.0 を返す関数があるならこれでOK
    return 1.0f; // API不明時は無効扱い
}

void Boss2RotatingPart::UpdateHpDrivenPattern()
{
    float r = GetBossHpRate(); // 0..1
    int new_phase = 0;
    if (r <= hp_t3) new_phase = 3;
    else if (r <= hp_t2) new_phase = 2;
    else if (r <= hp_t1) new_phase = 1;
    else new_phase = 0;

    if (new_phase != hp_phase)
    {
        hp_phase = new_phase;
        // フェーズごとに“らしい”パターンへ
        switch (hp_phase)
        {
            case 0: ForcePattern(BulletPattern::Spiral);     break; // 高体力：ライト
            case 1: ForcePattern(BulletPattern::Pendulum);       break;
            case 2: ForcePattern(BulletPattern::Spiral); break; // 中体力：追尾混ぜ
            case 3: ForcePattern(BulletPattern::NWay);      break; // 瀕死：密度UP
        }
    }
}

// ===== プレイヤー位置取得 =====

bool Boss2RotatingPart::TryGetPlayerPos(Vector2D& out) const
{
    // ADJUST HERE: プロジェクトのプレイヤー取得方法に合わせて1行で取得する
    // 例1) auto* p = Singleton<GameObjectManager>::GetInstance()->GetPlayer();
    // 例2) auto* p = Singleton<GameObjectManager>::GetInstance()->FindObject<Player>();
    // 例3) auto* p = boss ? boss->GetPlayer() : nullptr;
    // 取得できたら out = p->GetLocation(); return true; を返す

    return false;
}
