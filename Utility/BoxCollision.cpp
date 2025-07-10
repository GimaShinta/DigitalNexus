#include "BoxCollision.h"

/// <summary>
/// 適用オブジェクトか確認する処理
/// </summary>
/// <param name="hit_object">相手のオブジェクトタイプ</param>
/// <returns>適用するオブジェクトなら、true</returns>
bool BoxCollision::IsCheckHitTarget(eObjectType hit_object) const
{
    // 適用するオブジェクトタイプなら、true
    for (eObjectType type : hit_object_type)
    {
        if (type == hit_object)
        {
            return true;
        }
    }

    return false;
}

/// <summary>
/// 矩形の辺の関係位置で当たり判定をチェック
/// </summary>
/// <param name="c1">プレイヤー</param>
/// <param name="c2">敵</param>
/// <returns>当たっていたらtrue、当たっていなかったらfalse</returns>
bool IsCheckCollision(const BoxCollision& c1, const BoxCollision& c2)
{
    // 矩形1の左辺と矩形2の右辺の位置関係
    bool is_left_less_right = false;
    if (c1.point[0].x <= c2.point[1].x)
    {
        is_left_less_right = true;
    }

    // 矩形1の右辺と矩形2の左辺の位置関係
    bool is_right_greater_left = false;
    if (c1.point[1].x >= c2.point[0].x)
    {
        is_right_greater_left = true;
    }

    // 矩形1の上辺と矩形2の下辺の位置関係
    bool is_top_less_bottom = false;
    if (c1.point[0].y <= c2.point[1].y)
    {
        is_top_less_bottom = true;
    }

    // 矩形1の下辺と矩形2の上辺の位置関係
    bool is_bottom_greater_top = false;
    if (c1.point[1].y >= c2.point[0].y)
    {
        is_bottom_greater_top = true;
    }

    // 全ての条件が当てはまっていたら、ヒット
    if (is_left_less_right == true &&
        is_right_greater_left == true &&
        is_top_less_bottom == true &&
        is_bottom_greater_top == true)
    {
        return true;
    }

    // 条件が1つでも合っていなかったら、ノーヒット
    return false;
}