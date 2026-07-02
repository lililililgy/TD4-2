using System;

// アニメーションなしの追尾移動コントローラー
public class ChaseController : MonoScript
{
    [SerializeField] private float  chaseSpeed       = 5.0f;
    [SerializeField] private float  chaseDuration    = 3.0f;
    [SerializeField] private float  chaseDistance    = 5.0f;
    [SerializeField] private float  leachDistance   = 10.0f; 
    [SerializeField] private float  rushDistance     = 2.0f;
    [SerializeField] private string targetEntityName = "Player";
    [SerializeField] private float  waitTime         = 2.0f;
    [SerializeField] private bool   autoStart        = true;

    // --- 挙動フラグ ---
    // 進行方向を向くか
    [SerializeField] private bool enableFacing = true;
    // rushDistance 以内で停止するか
    [SerializeField] private bool stopAtRushDistance = false;

    // ChaseAnimationのリアクション中に待機する時間
    public float DiscoveryDuration { get; set; } = 0.0f;

    public enum State { Idle, Wait, Discovery, Chase, Rush }
    public State CurrentState { get; private set; } = State.Idle;

    // 状態変化イベント
    public Action OnWaitStart;
    public Action OnDiscoveryStart;
    public Action OnChaseStart;
    public Action OnRushStart;

    public Vector3 Velocity => velocity_;

    private Vector3 velocity_       = Vector3.zero;
    private float   chaseTimer_     = 0.0f;
    private float   waitTimer_      = 0.0f;
    private float   discoveryTimer_ = 0.0f;
    private bool    isFirstWait_    = true;
    private Entity  targetEntity_   = null;
    private Action  updateAction_   = null;
    private bool    isPaused_       = false;

    public override void Initialize()
    {
        // 自動開始フラグが立っていればチェイス開始
        if (autoStart)
        {
            StartChase();
        }
    }

    public override void Update()
    {
        // 攻撃中などは移動・状態タイマーごと停止する
        if (isPaused_)
        {
            velocity_ = Vector3.zero;
            return;
        }

        // 現在の状態 Action を実行
        updateAction_?.Invoke();
    }

    // 攻撃中など、外部から追跡を一時停止/再開させる
    public void SetPaused(bool paused)
    {
        isPaused_ = paused;
        if (paused)
        {
            velocity_ = Vector3.zero;
        }
    }

    public void StartChase()
    {
        // タイマーリセットして Wait へ
        chaseTimer_  = 0.0f;
        waitTimer_   = 0.0f;
        isFirstWait_ = true;
        TransitionTo(State.Wait);
    }

    private void TransitionTo(State next)
    {

        // Stateの遷移処理
        CurrentState = next;
        switch (next)
        {
            case State.Wait:
                // 停止してイベント発火
                velocity_     = Vector3.zero;
                updateAction_ = UpdateWait;
                OnWaitStart?.Invoke();
                break;
            case State.Discovery:
                // タイマーリセットしてイベント発火
                discoveryTimer_ = 0.0f;
                updateAction_   = UpdateDiscovery;
                OnDiscoveryStart?.Invoke();
                break;
            case State.Chase:
                // タイマーリセットしてイベント発火
                chaseTimer_   = 0.0f;
                updateAction_ = UpdateChase;
                OnChaseStart?.Invoke();
                break;
            case State.Rush:
                // イベント発火
                updateAction_ = UpdateRush;
                OnRushStart?.Invoke();
                break;
        }
    }

    private void UpdateWait()
    {
        // ターゲット未検出なら何もしない
        if (!TryFindTarget()) { return; }

        // 初回は waitTime をスキップ
        if (!isFirstWait_)
        {
            waitTimer_ += Time.deltaTime;
        }

        // 範囲内かつ待機時間経過で発見モーションへ
        Vector3 toTarget = targetEntity_.transform.position - transform.position;
        if (toTarget.Length() <= chaseDistance && (isFirstWait_ || waitTimer_ >= waitTime))
        {
            isFirstWait_ = false;
            TransitionTo(State.Discovery);
        }
    }

    private void UpdateDiscovery()
    {
        // 移動せずプレイヤーの方向を向いて待機
        CalcVelocityToTarget();
        TryFaceVelocity();

        // タイマー更新、演出時間が経過したら Chase へ
        discoveryTimer_ += Time.deltaTime;
        if (discoveryTimer_ >= DiscoveryDuration)
        {
            TransitionTo(State.Chase);
        }
    }

    private void UpdateChase()
    {
        // ターゲット未検出なら何もしない
        if (!TryFindTarget()) { return; }

        // 追いかけ時間切れなら Wait へ
        if (TickChaseTimer()) { return; }

        // 近距離判定
        Vector3 toTarget = targetEntity_.transform.position - transform.position;
        float   distance = toTarget.Length();

        // 離れすぎたら追跡を諦めて Wait へ
        if (ChangeWaitByDistance(distance)) { return; }

        if (distance <= rushDistance)
        {
            if (stopAtRushDistance)
            {
                // 指定距離で停止
                velocity_ = Vector3.zero;
            }
            else
            {
                // 方向固定で突進
                TransitionTo(State.Rush);
            }
            return;
        }

        // ターゲットへ向かって移動
        CalcVelocityToTarget();
        transform.position += velocity_ * Time.deltaTime;
        TryFaceVelocity();
    }

    private void UpdateRush()
    {
        // 追いかけ時間切れなら Wait へ
        if (TickChaseTimer()) { return; }

        // 方向固定で直進
        transform.position += velocity_ * Time.deltaTime;
        TryFaceVelocity();
    }

    private void TryFaceVelocity()
    {
        // フラグが無効なら向きを変えない
        if (!enableFacing) { return; }

        // 速度ゼロなら向きを変えない
        if (velocity_.x == 0.0f && velocity_.y == 0.0f) { return; }

        // 進行方向へ Slerp で回転
        Quaternion targetRotate = Quaternion.LookRotation(-Vector3.forward, velocity_.Normalized());
        transform.rotate = Quaternion.Slerp(transform.rotate, targetRotate, 0.3f);
    }

    private bool TryFindTarget()
    {
        // 未キャッシュなら検索
        if (targetEntity_ == null)
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }

        // 見つからなければ false
        return targetEntity_ != null;
    }

    private void CalcVelocityToTarget()
    {
        if (targetEntity_ == null) { return; }

        // ターゲット方向の速度ベクトルを計算
        Vector3 toTarget = targetEntity_.transform.position - transform.position;
        if (toTarget.Length() > 0.001f)
        {
            velocity_ = toTarget.Normalized() * chaseSpeed;
        }
    }

    private bool TickChaseTimer()
    {
        // 追いかけタイマーを更新
        chaseTimer_ += Time.deltaTime;

        // 時間切れで Wait へ
        if (chaseTimer_ >= chaseDuration)
        {
            velocity_    = Vector3.zero;
            waitTimer_   = 0.0f;
            isFirstWait_ = false;
            TransitionTo(State.Wait);
            return true;
        }
        return false;
    }

 
    private bool ChangeWaitByDistance(float distance)
    {
        if (distance >= leachDistance) { return false; }

        velocity_    = Vector3.zero;
        waitTimer_   = 0.0f;
        isFirstWait_ = false;
        TransitionTo(State.Wait);
        return true;
    }
}
