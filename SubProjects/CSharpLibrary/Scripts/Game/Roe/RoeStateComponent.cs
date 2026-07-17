using System;

// 卵(roe)1個の状態。卵プレハブ側に付ける。成熟したかどうかが唯一の真実(single source of truth)で、
// 残機数・弾数は RoeManager がこの状態を数えて導出する。
//
// 未成熟 → 成熟は自分で成長する（本来は経験値、暫定で時間経過）。
// 成熟した卵はそのまま弾（＝残機＝HP）としてカウントされる。
public class RoeStateComponent : MonoScript
{
    [SerializeField] private float minScale_ = 32.0f;  // 成長前のスケール
    [SerializeField] private float maxScale_ = 64.0f;  // 成熟後のスケール

    private bool isMature_ = false;

    public override void Initialize()
    {
        isMature_ = false;
        ApplyScale(minScale_);

        SpriteRenderer spriteRenderer = entity.GetComponent<SpriteRenderer>();
        if (spriteRenderer)
        {
            UVTransform uVTransform = spriteRenderer.uvTransform;
            uVTransform.position = new Vector2(0.5f, 0.5f);

            spriteRenderer.uvTransform = uVTransform;
        }
    }

    public override void Update()
    {
        // 成長するのは未成熟の間だけ。成熟後は最大スケール固定。
        if (isMature_)
        {
            ApplyScale(maxScale_);
            return;
        }

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();
        ApplyScale(Mathf.Lerp(minScale_, maxScale_, levelingComponent.GetExpProgress()));

        // Level UP したら成熟させる。見た目もここで合わせる。
        if (levelingComponent.IsLevelUp)
        {
            isMature_ = true;
            ApplyScale(maxScale_);

            SpriteAnimation spriteAnimationComponent = entity.GetScript<SpriteAnimation>();
            if (spriteAnimationComponent)
            {
                spriteAnimationComponent.Play();
            }
        }
    }

    public bool IsMature => isMature_;

    private void ApplyScale(float s)
    {
        Transform transform = entity.GetComponent<Transform>();
        if (transform)
        {
            transform.scale = Vector3.one * s;
        }
    }
}
