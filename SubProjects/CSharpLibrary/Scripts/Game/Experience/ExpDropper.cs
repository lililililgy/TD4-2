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


    /// ドロップする ExperiencePoint の量
    [SerializeField] private float dropExp = 0.0f;
    /// ダメージを受けたときにドロップする ExperiencePoint の量
    [SerializeField] private float damageDropExp = 0.0f;

    // 破棄理由の判別に使う。OnDestroy 時に native を引き直さずに済むよう Initialize で持つ。
    private HP hp_;

    public override void Initialize()
    {
        hp_ = entity.GetScript<HP>();
    }

    public override void OnDestroy()
    {
        // 撃破されたときだけ撒く。
        // entity.Destroy() は撃破以外の経路からも呼ばれるため、OnDestroy だけでは区別できない:
        //   ・DespawnTimer の時間切れ  ・SeaCow の自爆  ・シーン遷移時の一括破棄
        // いずれも HP を経由しないので、HP.IsDead が「倒された」の唯一の証拠になる。
        // HP を持たない entity はそもそも撃破され得ないので落とさない。
        if (hp_ == null)
        {
            hp_ = entity.GetScript<HP>();
        }
        if (hp_ == null || !hp_.IsDead)
        {
            return;
        }

        Drop();
    }

    /// <summary>
    /// ダメージを受けたときの ExperiencePoint をドロップする
    /// </summary>
    public void DropOnDamage()
    {
        Drop(damageDropExp);
    }

    /// <summary>
    /// 指定された ExperiencePoint の分だけ ExpOrb をばらまく
    /// </summary>
    private void Drop(float exp)
    {
        if (exp <= 0.0f)
        {
            return;
        }

        float remainingExp = exp;

        // 大きいオーブから優先的に割り当てる整数個数の分解（例: exp=235 → large 2, medium 3, small 5）
        int largeCount = (int)(remainingExp / kLargeOrbExp);
        remainingExp -= largeCount * kLargeOrbExp;
        int mediumCount = (int)(remainingExp / kMediumOrbExp);
        remainingExp -= mediumCount * kMediumOrbExp;
        int smallCount = (int)(remainingExp / kSmallOrbExp);

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
        Vector2 scale = transform != null ? new Vector2(transform.scale.x, transform.scale.y) : Vector2.one;
        Vector2 half = Vector2.zero;

        BoxCollider2D boxCollider2D = entity != null ? entity.GetComponent<BoxCollider2D>() : null;
        if (boxCollider2D != null)
        {
            half = new Vector2(boxCollider2D.size.x * scale.x, boxCollider2D.size.y * scale.y) * 0.5f;
        }
        else
        {
            CircleCollider circleCollider = entity != null ? entity.GetComponent<CircleCollider>() : null;
            if (circleCollider != null)
            {
                half = new Vector2(circleCollider.radius * scale.x, circleCollider.radius * scale.y);
            }
        }

        Vector2 center = transform != null ? new Vector2(transform.position.x, transform.position.y) : Vector2.zero;
        min = center - half;
        max = center + half;
    }

    private void SpawnOrbs(string prefabName, int count, Vector2 min, Vector2 max)
    {
        if (ecsGroup == null) return;
        for (int i = 0; i < count; i++)
        {
            Entity created = ecsGroup.CreateEntity(prefabName);
            if (created == null || created.transform == null)
            {
                continue;
            }
            created.transform.position = new Vector3(
                RandomUtil.RandomRange(min.x, max.x),
                RandomUtil.RandomRange(min.y, max.y),
                0
            );
        }
    }
}
