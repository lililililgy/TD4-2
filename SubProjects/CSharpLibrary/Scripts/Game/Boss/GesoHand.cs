using System;

public class GesoHand : MonoScript
{
    // 手の状態を表す列挙型
    public enum HandState
    {
        Idle, // 待機状態
        Aiming, // 狙っている状態
        Attacking, // 攻撃中の状態
        Returning, // 手が元の位置に戻る状態
        Damageing, // ダメージを受けている状態
    }

    [SerializeField]
    public float rotationSpeed = 8.0f;　//回転速度
    [SerializeField]
    public float attackDamage = 1.0f; //攻撃力
    [SerializeField]
    public float attackRadius = 1.5f; //攻撃範囲
    [SerializeField]
    public float attackDuration = 0.4f; //攻撃の持続時間
    [SerializeField]
    public float moveDuration = 0.25f; //ターゲットまで直線移動する時間
    [SerializeField]
    public float passThroughDistance = 500.0f; //ターゲットを通過して進む距離
    [SerializeField]
    public float returnDuration = 0.3f; //手が元の位置に戻るまでの時間

    // 現在の手の状態
    public HandState State { get; private set; }
    // 攻撃可能かどうかを判定するプロパティ
    public bool CanAttack { get { return State == HandState.Idle || State == HandState.Aiming; } }

    // 手の元の回転
    private Quaternion _homeRotation;
    private Vector2 _homePosition;
    private Vector2 _attackTargetPosition;
    private float _movementDepth;
    // 攻撃対象のエンティティ
    private Entity _target;
    // 現在の状態の経過時間
    private float _stateTime;
    // 攻撃イベントが送信されたかどうかを追跡するフラグ
    private bool _attackEventSent;

    //弱点インスタンス
    private GesoWeakPoint _weakPoint;

    //=============================
    // 初期化
    //=============================
    public override void Initialize()
    {
        _homeRotation = transform.rotation;
        _homePosition = ToPlane(transform.position);
        _movementDepth = transform.position.z;
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
        if (!CanAttack || target == null || target.transform == null)
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
        if (!CanAttack || target == null || target.transform == null)
        {
            return false;
        }

        _target = target;
        Vector2 targetPosition = ToPlane(target.transform.position);
        Vector2 attackDirection = (targetPosition - _homePosition).Normalized();
        _attackTargetPosition = targetPosition + attackDirection * passThroughDistance;
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

        // 攻撃開始時に記録した位置へ向きを合わせる
        RotateTowardPosition(_attackTargetPosition);

        float duration = moveDuration > 0.0f ? moveDuration : 0.001f;
        float moveRatio = Mathf.Clamp01(_stateTime / duration);
        SetPlanePosition(Lerp(_homePosition, _attackTargetPosition, moveRatio));

        if (!_attackEventSent && moveRatio >= 1.0f)
        {
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
        float returnRatio = Mathf.Clamp01(Time.deltaTime / duration);
        Vector2 currentPosition = ToPlane(transform.position);
        SetPlanePosition(Lerp(currentPosition, _homePosition, returnRatio));
        transform.rotation = Quaternion.Slerp(transform.rotation, _homeRotation, returnRatio);

        if (_stateTime >= returnDuration)
        {
            transform.rotation = _homeRotation;
            SetPlanePosition(_homePosition);
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

        RotateTowardPosition(ToPlane(_target.transform.position));
    }

    //============================
    // ターゲットの位置に向かって回転する処理
    //============================
    private void RotateTowardPosition(Vector2 targetPosition)
    {
        Vector2 direction = targetPosition - ToPlane(transform.position);
        if (direction.Length() <= 0.001f)
        {
            return;
        }

        Vector2 normalized = direction.Normalized();
       float angle = Mathf.Atan2(normalized.y, normalized.x) * Mathf.Rad2Deg;
        transform.rotation = Quaternion.MakeFromAxis(Vector3.forward, angle);
    }

    private static Vector2 ToPlane(Vector3 position)
    {
        return new Vector2(position.x, position.y);
    }

    private void SetPlanePosition(Vector2 position)
    {
        transform.position = new Vector3(position.x, position.y, _movementDepth);
    }

    private static Vector2 Lerp(Vector2 start, Vector2 end, float ratio)
    {
        return start + (end - start) * ratio;
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
