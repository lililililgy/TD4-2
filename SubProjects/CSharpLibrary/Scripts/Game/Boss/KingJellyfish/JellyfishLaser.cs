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
    private Vector2 origin_;
    private Vector2 direction_ = Vector2.up;
    private float depth_;
    private float rotationSpeed_;
    private bool configured_;
    private bool geometryApplied_;

    public override void Initialize()
    {
        // Configureは生成フレーム、Initializeは次フレームに呼ばれる。
        // 発射設定を受け取り済みなら、その設定を維持する。
        if (!configured_)
        {
            elapsed_ = 0.0f;
            geometryApplied_ = false;
        }
    }

    public override void Update()
    {
        if (!configured_)
        {
            return;
        }

        if (!geometryApplied_)
        {
            // レーザーの形状を子オブジェクトに適用する
            geometryApplied_ = ApplyGeometryToChild();
            if (!geometryApplied_)
            {
                return;
            }
        }

        elapsed_ += Time.deltaTime;
        if (rotationSpeed_ != 0.0f)
        {
            Quaternion frameRotation = Quaternion.MakeFromAxis(Vector3.back, rotationSpeed_ * Time.deltaTime);
            transform.rotation = frameRotation * transform.rotation;
        }

        if (elapsed_ >= duration)
        {
            entity.Destroy();
        }
    }

    //==========================================
    // レーザーの設定処理
    //==========================================
    public void Configure(
        Vector2 origin,
        Vector2 direction,
        float laserLength,
        float laserWidth,
        float laserDamage,
        float laserDuration,
        float depth,
        float rotationSpeed)
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

        origin_ = origin;
        direction_ = normalized;
        depth_ = depth;
        rotationSpeed_ = rotationSpeed;
        elapsed_ = 0.0f;
        configured_ = true;
        geometryApplied_ = false;
    }

    private bool ApplyGeometryToChild()
    {
        Entity laserBody = entity.GetChild(0);
        if (laserBody == null || laserBody.transform == null)
        {
            return false;
        }

        float angle = Mathf.Atan2(direction_.x, direction_.y);

        transform.position = new Vector3(origin_.x, origin_.y, depth_);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
        transform.scale = new Vector3(1.0f, 1.0f, 1.0f);

        laserBody.transform.position = new Vector3(0.0f, length * 0.5f, 0.0f);
        laserBody.transform.rotation = Quaternion.identity;
        laserBody.transform.scale = new Vector3(width, length, 1.0f);

        AttackCollision attackCollision = laserBody.GetScript<AttackCollision>();
        if (attackCollision != null)
        {
            attackCollision.Damage = damage;
        }

        return true;
    }
}
