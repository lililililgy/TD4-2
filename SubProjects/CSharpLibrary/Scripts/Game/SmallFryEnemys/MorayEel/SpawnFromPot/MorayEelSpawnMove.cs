using System;


public class MorayEelSpawnMove : MonoScript
{

    [SerializeField] private float deraySpeed = 1.0f;
    [SerializeField] private float launchTime = 1.0f;
    [SerializeField] private string playerEntityName = "Player";
    [SerializeField] private float spawnSpinCount = 3.0f;

    // velocity
    private Vector3 velocity_ = Vector3.zero;
    // 出現からの経過時間
    private float timer_ = 0.0f;
    // 発射フラグ
    private bool launched_ = false;
    //Action,PlayerEntity
    private Action updateAction_;
    private Entity playerEntity_ = null;
    // 雑魚敵の追いかけスクリプト
    private SmallFryChaseMove chaseMove_;
    // 目標角度
    private Quaternion targetRotation_;
    private RigidbodyMotion motion_ = new RigidbodyMotion(6.0f);

    public override void Initialize()
    {
        motion_.Attach(entity);

        // 追いかけMoveスクリプトを取得
        chaseMove_ = entity.GetScript<SmallFryChaseMove>();

        // 発射時の向きを終着点として保存
        targetRotation_ = transform.rotate;

        // ツボから出てきた場合は、SpawnMoveに遷移
        if (!launched_)
        {
            velocity_ = Vector3.zero;
            timer_ = 0.0f;
            updateAction_ = SpawnMove;
        }
    }

    public override void Update()
    {
        // Actionの更新
        updateAction_?.Invoke();
    }

    private void SpawnMove()
    {

        // タイマー更新
        timer_ += Time.deltaTime;

        // スピードを減衰しつつ、velocity方向へ進
        float speed = velocity_.Length();
        speed = Math.Max(0.0f, speed - deraySpeed * Time.deltaTime);
        velocity_ = velocity_.Normalized() * speed;

        // 位置の適応
        motion_.Apply(transform, velocity_);

        // -------------登場時の回転演出
        // tの計算
        float t = Math.Min(timer_ / launchTime, 1.0f);
        // 序盤は速く回り、終盤に向けて減速
        float remaining = Ease.In.Quad(1.0f - t);
        // 回転させて、適用する
        float extraAngle = spawnSpinCount * 2.0f * (float)Math.PI * remaining;
        transform.rotate = Quaternion.MakeFromAxis(Vector3.forward, extraAngle) * targetRotation_;

        // 時間経過でChargeMoveに移行
        if (timer_ >= launchTime)
        {
            transform.rotate = targetRotation_;
            updateAction_ = ChargeMove;
        }
    }

    private void ChargeMove()
    {

        // Playerエンティティを取得
        if (playerEntity_ == null)
        {
            playerEntity_ = ecsGroup.FindEntity(playerEntityName);
        }

        // Playerエンティティnullチェック
        if (playerEntity_ == null)
        {
            return;
        }


        chaseMove_?.StartChase();
        updateAction_ = null;
        return;
    }

    /// <summary>ツボから発射された瞬間にツボ側から呼ばれる。</summary>
    public void Launch(Vector3 initialVelocity)
    {
        velocity_ = initialVelocity;
        timer_ = 0.0f;
        launched_ = true;
        updateAction_ = SpawnMove;
    }
}
