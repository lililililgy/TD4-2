using System;
using System.Collections.Generic;

/// <summary>
/// Experience point dropper
/// Experience point を ExpOrb として ドロップするためのクラス
/// </summary>
public class ExpDropper : MonoScript
{
    private const float kSmallOrbExp = 1.0f;
    private const float kMediumOrbExp = 10.0f;
    private const float kLargeOrbExp = 100.0f;

    private const string kSmallOrbPrefabName = "ExpOrbSmall";
    private const string kMediumOrbPrefabName = "ExpOrbMedium";
    private const string kLargeOrbPrefabName = "ExpOrbLarge";


    public override void OnDestroy()
    {
        Drop();
    }

    /// <summary>
    /// 保持している ExperiencePoint の分だけ ExpOrb をばらまく
    /// </summary>
    private void Drop()
    {
        ExperiencePoint expp = entity.GetScript<ExperiencePoint>();
        if (expp == null)
        {
            return;
        }

        float exp = expp.ExperiencePoints;

        // 大きいオーブから優先的に割り当てる整数個数の分解（例: exp=235 → large 2, medium 3, small 5）
        int largeCount = (int)(exp / kLargeOrbExp);
        exp -= largeCount * kLargeOrbExp;
        int mediumCount = (int)(exp / kMediumOrbExp);
        exp -= mediumCount * kMediumOrbExp;
        int smallCount = (int)exp; // kSmallOrbExp == 1 なので端数がそのまま個数になる

        Vector2 min, max;
        GetSpawnArea(out min, out max);

        SpawnOrbs(kLargeOrbPrefabName, largeCount, min, max);
        SpawnOrbs(kMediumOrbPrefabName, mediumCount, min, max);
        SpawnOrbs(kSmallOrbPrefabName, smallCount, min, max);
    }

    /// <summary>
    /// Collider の形状からオーブをばらまく矩形範囲を求める
    /// BoxCollider2D / CircleCollider のどちらも中心オフセットを持たないため、
    /// Entity の位置を中心とみなす
    /// </summary>
    private void GetSpawnArea(out Vector2 min, out Vector2 max)
    {
        Vector2 scale = new Vector2(transform.scale.x, transform.scale.y);
        Vector2 half = Vector2.zero;

        BoxCollider2D boxCollider2D = entity.GetComponent<BoxCollider2D>();
        if (boxCollider2D != null)
        {
            half = new Vector2(boxCollider2D.size.x * scale.x, boxCollider2D.size.y * scale.y) * 0.5f;
        }
        else
        {
            CircleCollider circleCollider = entity.GetComponent<CircleCollider>();
            if (circleCollider != null)
            {
                half = new Vector2(circleCollider.radius * scale.x, circleCollider.radius * scale.y);
            }
        }

        Vector2 center = new Vector2(transform.position.x, transform.position.y);
        min = center - half;
        max = center + half;
    }

    private void SpawnOrbs(string prefabName, int count, Vector2 min, Vector2 max)
    {
        for (int i = 0; i < count; i++)
        {
            Entity created = ecsGroup.CreateEntity(prefabName);
            created.transform.position = new Vector3(
                RandomUtil.RandomRange(min.x, max.x),
                RandomUtil.RandomRange(min.y, max.y),
                0
            );
        }
    }
}
