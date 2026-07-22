using System;

///=============================================
/// ボスの弱点を表すクラス
///=============================================
public class JellyfishWeakPoint : MonoScript
{

    // ボスのエンティティ名
    [SerializeField]
    private string kingJellyfishEntityName = "KingJellyfish";

    // ボスの参照
    private KingJellyfish kingJellyfish;
    private Rigidbody2D rigidbody_;

    //=============================
    // 初期化
    //=============================
    public override void Initialize()
    {
        rigidbody_ = entity.GetComponent<Rigidbody2D>();
        SetVelocity(Vector2.zero);
        ResolveKingJellyfish();
    }

    //=============================
    // 更新
    //=============================
    public override void Update()
    {

    }

    //=============================
    // 弱点の衝突判定を切り替える
    //=============================
    public void SetCollisionEnabled(bool enabled)
    {
        BoxCollider2D collider = entity.GetComponent<BoxCollider2D>();
        if (collider != null)
        {
            collider.enable = enabled ? 1 : 0;
        }
    }

    public void SetVelocity(Vector2 velocity)
    {
        if (rigidbody_ == null)
        {
            rigidbody_ = entity.GetComponent<Rigidbody2D>();
        }

        if (rigidbody_ != null)
        {
            rigidbody_.velocity = velocity;
        }
    }

    //=============================
    // ダメージを与える
    //=============================
    public void Damage(int damage)
    {
        Damage((float)damage);
    }


    public void Damage(float damage)
    {
        if (kingJellyfish == null)
        {
            // ボスの参照がない場合は再取得
            ResolveKingJellyfish();
        }

        if (kingJellyfish != null)
        {
            // ボスにダメージを与える
            kingJellyfish.TakeDamage(damage);
        }
    }

    //=============================
    // ボスの参照を解決する
    //=============================
    private void ResolveKingJellyfish()
    {
        kingJellyfish = null;

        // ECSグループを取得
        if (!String.IsNullOrEmpty(kingJellyfishEntityName))
        {
            Entity kingJellyfishEntity = ecsGroup.FindEntity(kingJellyfishEntityName);
            //取得できなかった場合はnullを返す
            kingJellyfish = kingJellyfishEntity != null ? kingJellyfishEntity.GetScript<KingJellyfish>() : null;
        }

        if (kingJellyfish == null)
        {
            // 取得できなかった場合は、親のエンティティから取得を試みる
            kingJellyfish = entity.GetScript<KingJellyfish>();
        }
    }
}
