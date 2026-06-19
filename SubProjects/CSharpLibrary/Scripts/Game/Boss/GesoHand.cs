using System;

public class GesoHand : MonoScript
{
    // 手の状態を表す列挙型
    public enum HandState
    {
        Idle,
        Aiming,
        Attacking,
        Returning,
    }

    [SerializeField]
    public float rotationSpeed = 8.0f;　//回転速度
    [SerializeField]
    public float attackDamage = 10.0f; //攻撃力
    [SerializeField]
    public float attackRadius = 1.5f; //攻撃範囲
    [SerializeField]
    public float attackDuration = 0.4f; //攻撃の持続時間
    [SerializeField]
    public float returnDuration = 0.3f; //手が元の位置に戻るまでの時間
    [SerializeField]
    public float attackOffsetForward = 2.0f; //攻撃の前方オフセット
    [SerializeField]
    public float attackOffsetUp = 0.0f; //攻撃の上方向オフセット

    // 現在の手の状態
    public HandState State { get; private set; }
    // 攻撃可能かどうかを判定するプロパティ
    public bool CanAttack { get { return State == HandState.Idle || State == HandState.Aiming; } }

    // 手の元の回転
    private Quaternion _homeRotation;
    // 攻撃対象のエンティティ
    private Entity _target;
    // 現在の状態の経過時間
    private float _stateTime;
    // 攻撃イベントが送信されたかどうかを追跡するフラグ
    private bool _attackEventSent;

    //=============================
    // 初期化
    //=============================
    public override void Initialize()
    {
        _homeRotation = transform.rotation;
        State = HandState.Idle;
        _target = null;
        _stateTime = 0.0f;
        _attackEventSent = false;
    }

    //=============================
    // 更新
    //=============================
    public override void Update()
    {
        switch (State)
        {
            case HandState.Aiming:
                RotateTowardTarget();
                break;

            case HandState.Attacking:
                UpdateAttack();
                break;

            case HandState.Returning:
                UpdateReturn();
                break;
        }
    }

    //=============================
    // プレイヤーを狙う処理
    //=============================
    public void CommandAim(Entity target)
    {
        if (!CanAttack || target == null)
        {
            return;
        }

        _target = target;
        State = HandState.Aiming;
    }

    //=============================
    // 攻撃処理
    //=============================
    public bool CommandAttack(Entity target)
    {
        if (!CanAttack || target == null)
        {
            return false;
        }

        _target = target;
        _stateTime = 0.0f;
        _attackEventSent = false;
        State = HandState.Attacking;
        return true;
    }

    //=============================
    // 手を元の位置に戻す処理
    //=============================
    public void CommandIdle()
    {
        if (State == HandState.Aiming)
        {
            BeginReturn();
        }
    }

    //=============================
    // 攻撃中の更新処理
    //=============================
    private void UpdateAttack()
    {
        if (_target == null || _target.transform == null)
        {
            BeginReturn();
            return;
        }

        RotateTowardTarget();

        if (!_attackEventSent)
        {
            FrameEvent.EnqueueAttackEvent(
                "GesoHandAttack",
                entity.Id,
                attackDamage,
                attackRadius,
                attackDuration,
                attackOffsetForward,
                attackOffsetUp);
            _attackEventSent = true;
        }

        _stateTime += Time.deltaTime;
        if (_stateTime >= attackDuration)
        {
            BeginReturn();
        }
    }

    //=============================
    // 手を元の位置に戻す更新処理
    //=============================
    private void UpdateReturn()
    {
        _stateTime += Time.deltaTime;
        float duration = returnDuration > 0.0f ? returnDuration : 0.001f;
        transform.rotation = Quaternion.Slerp(transform.rotation, _homeRotation, Mathf.Clamp01(Time.deltaTime / duration));

        if (_stateTime >= returnDuration)
        {
            transform.rotation = _homeRotation;
            _target = null;
            State = HandState.Idle;
        }
    }

    //=============================
    // ターゲットに向かって回転する処理
    //=============================
    private void RotateTowardTarget()
    {
        if (_target == null || _target.transform == null)
        {
            BeginReturn();
            return;
        }

        Vector3 direction = _target.transform.worldPosition - transform.worldPosition;
        if (direction.Length() <= 0.001f)
        {
            return;
        }

        Quaternion targetRotation = Quaternion.LookRotation(direction.Normalized());
        transform.rotation = Quaternion.Slerp(
            transform.rotation,
            targetRotation,
            Mathf.Clamp01(rotationSpeed * Time.deltaTime));
    }

    //=============================
    // 手を元の位置に戻す処理を開始する
    //=============================
    private void BeginReturn()
    {
        _stateTime = 0.0f;
        State = HandState.Returning;
    }
}
