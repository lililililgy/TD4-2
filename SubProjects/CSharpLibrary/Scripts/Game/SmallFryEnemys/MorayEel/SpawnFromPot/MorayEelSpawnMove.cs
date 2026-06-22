using System;


public class MorayEelSpawnMove : MonoScript
{

    [SerializeField] private float deraySpeed = 1.0f;
    [SerializeField] private float launchTime = 1.0f;
    [SerializeField] private string playerEntityName = "Player";

    private Vector3 velocity_ = Vector3.zero;
    private float timer_ = 0.0f;
    private bool launched_ = false;
    private Action updateAction_;
    private Entity playerEntity_ = null;
    private SmallFryChaseMove chaseMove_;

    public override void Initialize()
    {
        // 追いかけMoveスクリプトを取得
        chaseMove_ = entity.GetScript<SmallFryChaseMove>();

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
        transform.position += velocity_ * Time.deltaTime;

        // 時間経過でChargeMoveに移行
        if (timer_ >= launchTime)
        {
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
