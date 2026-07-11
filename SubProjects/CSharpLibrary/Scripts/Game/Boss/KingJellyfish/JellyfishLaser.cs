using System;

//==========================================
// クラゲのレーザー攻撃クラス
//==========================================
public class JellyfishLaser : MonoScript
{
    [SerializeField] public float damage = 1.0f;
    [SerializeField] public float duration = 3.0f;
    [SerializeField] public float length = 48000.0f;
    [SerializeField] public float width = 100.0f;

    private float elapsed_;

    public override void Initialize()
    {
        elapsed_ = 0.0f;
    }

    public override void Update()
    {
        elapsed_ += Time.deltaTime;
        if (elapsed_ >= duration)
        {
            DestroyLaserRoot();
        }
    }

    private void DestroyLaserRoot()
    {
        Entity parent = entity.parent;
        if (parent != null)
        {
            parent.Destroy();
            return;
        }

        entity.Destroy();
    }

    //==========================================
    // レーザーの設定処理
    //==========================================
    public void Configure(Vector2 origin, Vector2 direction, float laserLength, float laserWidth, float laserDamage, float laserDuration, float depth)
    {
        // 正規化された方向ベクトルを取得
        Vector2 normalized = direction.Normalized();
        if (normalized.LengthSq() <= 0.001f)
        {
            normalized = Vector2.up;
        }

        length = laserLength;
        width = laserWidth;
        damage = laserDamage;
        duration = laserDuration > 0.0f ? laserDuration : 0.01f;
        elapsed_ = 0.0f;

        // レーザーの中心位置を計算
        Vector2 center = origin + normalized * (length * 0.5f);
        float angle = Mathf.Atan2(normalized.x, normalized.y);
        Quaternion rotation = Quaternion.MakeFromAxis(Vector3.back, angle);

        Entity parent = entity.parent;
        if (parent != null)
        {
            parent.transform.position = new Vector3(center.x, center.y, depth);
            parent.transform.rotation = rotation;
            parent.transform.scale = new Vector3(1.0f, 1.0f, 1.0f);

            transform.position = new Vector3(0.0f, 0.0f, 0.0f);
            transform.rotation = Quaternion.identity;
            transform.scale = new Vector3(width, length, 1.0f);
        }
        else
        {
            transform.position = new Vector3(center.x, center.y, depth);
            transform.rotation = rotation;
            transform.scale = new Vector3(width, length, 1.0f);
        }

        // 攻撃判定の設定
        AttackCollision attackCollision = entity.GetScript<AttackCollision>();
        if (attackCollision != null)
        {
            attackCollision.Damage = damage;
        }
    }
}
