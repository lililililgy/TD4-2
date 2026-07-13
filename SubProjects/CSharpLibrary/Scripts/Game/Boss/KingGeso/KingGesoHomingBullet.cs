
/// <summary>
/// キングゲソのホーミング弾
/// </summary>
public class KingGesoHomingBullet : MonoScript
{
    [SerializeField] public float speed = 120.0f;
    [SerializeField] public float turnSpeed = 2.0f;
    [SerializeField] public float lifeTime = 8.0f;
    [SerializeField] public float damage = 1.0f;

    private Entity target;
    private Vector3 direction = Vector3.up;
    private float elapsed;
    private bool launched;
    private bool destroyRequested;
    private bool configured;

    public override void Initialize()
    {
        // CommandLaunchは生成フレーム、Initializeは次フレームに呼ばれる。
        // 発射済みの設定をInitializeで上書きしない。
        if (!configured)
        {
            target = null;
            direction = Vector3.up;
            elapsed = 0.0f;
            launched = false;
            destroyRequested = false;
        }
    }

    //===========================================================
    // コマンド: 発射
    //===========================================================
    public bool CommandLaunch(Entity target)
    {
        if (target == null || target.transform == null)
        {
            return false;
        }

        // ターゲットの方向を向く
        this.target = target;
        Vector3 targetPosition = this.target.transform.position;
        Vector3 currentPosition = transform.position;
        Vector3 toTarget = new Vector3(
            targetPosition.x - currentPosition.x,
            targetPosition.y - currentPosition.y,
            0.0f);
        if (toTarget.LengthSq() > 0.0001f)
        {
            this.direction = toTarget.Normalized();
        }

        this.launched = true;
        this.configured = true;
        this.elapsed = 0.0f;
        this.destroyRequested = false;
        ApplyRotation();

        AttackCollision attack = entity.GetScript<AttackCollision>();
        if (attack != null)
        {
            attack.Damage = damage;
        }
        return true;
    }

    public override void Update()
    {
        if (this.destroyRequested)
        {
            entity.Destroy();
            return;
        }

        if (!this.launched)
        {
            return;
        }

        this.elapsed += Time.deltaTime;
        if (this.elapsed >= lifeTime)
        {
            entity.Destroy();
            return;
        }

        if (this.target != null && this.target.transform != null)
        {
            // ターゲットの方向に向かって回転する
            Vector3 targetPosition = this.target.transform.position;
            Vector3 currentPosition = transform.position;
            Vector3 toTarget = new Vector3(
                targetPosition.x - currentPosition.x,
                targetPosition.y - currentPosition.y,
                0.0f);
            if (toTarget.LengthSq() > 0.0001f)
            {
                Quaternion current = Quaternion.LookRotation(-Vector3.forward, this.direction);
                Quaternion desired = Quaternion.LookRotation(-Vector3.forward, toTarget.Normalized());
                float ratio = Mathf.Clamp(turnSpeed * Time.deltaTime, 0.0f, 1.0f);
                this.direction = Quaternion.RotateVector(Quaternion.Slerp(current, desired, ratio), Vector3.up).Normalized();
            }
        }

        // 移動
        transform.position += this.direction * speed * Time.deltaTime;
        ApplyRotation();
    }

    public override void OnCollisionEnter(Entity collision)
    {
        if (this.launched && collision != null)
        {
            this.destroyRequested = true;
        }
    }

    private void ApplyRotation()
    {
        float roll = Mathf.Atan2(this.direction.x, this.direction.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
    }
}
