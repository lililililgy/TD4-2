using System;


public class SmallFryChaseMove : MonoScript
{

    // チェイス系パラメータ
    [SerializeField] private float chaseSpeed = 5.0f;
    [SerializeField] private float chaseDuration = 3.0f;
    [SerializeField] private float chaseDistance = 5.0f;
    // プレイヤーへの向きを維持するまでの距離
    [SerializeField] private float rushDistance = 2.0f;
    // ターゲットEntity名
    [SerializeField] private string playerEntityName = "Player";

    // 発見演出のスケーリング
    [SerializeField] private float discoveryScale = 2.0f;
    [SerializeField] private float discoveryTimer = 0.5f;

    // Waitのパラメータ
    [SerializeField] private float waitTime          = 2.0f; // この時間が経過するまでChaseに移行しない
    // Wait中のパルス
    [SerializeField] private float waitPulseScale    = 1.2f;
    [SerializeField] private float waitPulseDuration = 1.0f;

    // チェイス中のパルス
    [SerializeField] private float chasePulseScale    = 1.4f;
    [SerializeField] private float chasePulseDuration = 0.4f;

    // 移動速度,追いかけタイマー
    private Vector3 velocity_ = Vector3.zero;
    private float chaseTimer_ = 0.0f;
    // 発見時のスケール、タイマー
    private float currentDiscoveryTimer_ = 0.0f;
    private Vector3 initialScale_ = new Vector3(1.0f, 1.0f, 1.0f);
    // パルスタイマー
    private float pulseTimer_ = 0.0f;
    // Waitタイマー
    private float waitTimer_    = 0.0f;
    private bool  isFirstWait_  = true; // 初回Waitはwaittimeを無視する
    // entity
    private Entity playerEntity_ = null;
    // action
    private Action updateAction_ = null;

    public override void Initialize()
    {
        initialScale_ = transform.scale;
    }

    public override void Update()
    {
        // Action更新
        updateAction_?.Invoke();
    }

    public void StartChase()
    {
        // チェイスActionに移行
        chaseTimer_  = 0.0f;
        pulseTimer_  = 0.0f;
        waitTimer_   = 0.0f;
        isFirstWait_ = true;
        updateAction_ = Wait;
    }


    private void ChaseMove()
    {

        // プレイヤーEntitynullチェック
        if (!TryFindPlayer()) { return; }

        // 追いかけタイマー更新
        if (TickChaseTimer()) { return; }

        // プレイヤーへのベクトルを取得
        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        float distance = toPlayer.Length();

        // 近づきすぎたら方向固定で直進
        if (distance <= rushDistance)
        {
            updateAction_ = RushMove;
            return;
        }

        // プレイヤーの方向へ追尾する
        CalcVelocityToPlayer();

        // 位置適応・進行方向へ向く
        transform.position += velocity_ * Time.deltaTime;
        FaceVelocity();

        // チェイス中パルス
        UpdatePulse(chasePulseScale, chasePulseDuration);
    }

    private void RushMove()
    {
        // 追いかけタイマー更新・時間切れで Wait へ
        if (TickChaseTimer())
        {
            return;
        }

        // 位置とvelocityの適用
        transform.position += velocity_ * Time.deltaTime;
        FaceVelocity();

        // チェイス中パルス
        UpdatePulse(chasePulseScale, chasePulseDuration);
    }

    private void FaceVelocity()
    {
        // 動いていなかったら早期リターン
        if (velocity_.x == 0.0f && velocity_.y == 0.0f)
        {
            return;
        }
        // 進行方向へのターゲット回転を計算し、現在角度から Slerp で補間
        Quaternion targetRotate = Quaternion.LookRotation(-Vector3.forward, velocity_.Normalized());
        transform.rotate = Quaternion.Slerp(transform.rotate, targetRotate, 0.3f);
    }

    private void Wait()
    {

        // プレイヤーEntityのnullチェック
        if (!TryFindPlayer())
        {
            return;
        }

        // Waitタイマー更新
        waitTimer_ += Time.deltaTime;

        // Waitタイマー更新（初回はスキップ）
        if (!isFirstWait_) { waitTimer_ += Time.deltaTime; }

        // 一定距離までプレイヤーがいて、かつ waitTime が経過したら発見モーションへ移行（初回は即時）
        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        if (toPlayer.Length() <= chaseDistance && (isFirstWait_ || waitTimer_ >= waitTime))
        {
            currentDiscoveryTimer_ = 0.0f;
            pulseTimer_ = 0.0f;
            transform.scale = initialScale_;
            updateAction_ = DiscoveryMotion;
            return;
        }

        // Wait中パルス
        UpdatePulse(waitPulseScale, waitPulseDuration);
    }

    private void DiscoveryMotion()
    {

        // プレイヤーの方向を計算する
        CalcVelocityToPlayer();
        FaceVelocity();

        // 発見演出タイマー更新
        currentDiscoveryTimer_ += Time.deltaTime;
        float halfTime = discoveryTimer * 0.5f;

        if (currentDiscoveryTimer_ < halfTime)
        {
            // 前半: EaseOutBack でスケールアップ
            float time = currentDiscoveryTimer_ / halfTime;
            float scale = 1.0f + (discoveryScale - 1.0f) * Ease.Out.Back(time);
            transform.scale = initialScale_ * scale;
        }
        else if (currentDiscoveryTimer_ < discoveryTimer)
        {
            // 後半: EaseOutBack で元のスケールに戻す
            float time = (currentDiscoveryTimer_ - halfTime) / halfTime;
            float scale = discoveryScale + (1.0f - discoveryScale) * Ease.Out.Back(time);
            transform.scale = initialScale_ * scale;
        }
        else
        {
            // 演出終了 → ChaseMove へ
            transform.scale = initialScale_;
            chaseTimer_ = 0.0f;
            pulseTimer_ = 0.0f;
            updateAction_ = ChaseMove;
        }
    }

    private void UpdatePulse(float maxScale, float duration)
    {
        pulseTimer_ += Time.deltaTime;

        // duration でループ
        float t = (pulseTimer_ % duration) / duration;
        float half = 0.5f;
        float scale;

        if (t < half)
        {
            // 前半: EaseOutBack でスケールアップ
            scale = 1.0f + (maxScale - 1.0f) * Ease.Out.Back(t / half);
        }
        else
        {
            // 後半: EaseOutBack で元のスケールに戻す
            scale = maxScale + (1.0f - maxScale) * Ease.Out.Back((t - half) / half);
        }

        transform.scale = initialScale_ * scale;
    }

    private bool TryFindPlayer()
    {
        // プレイヤーEntityを取得
        if (playerEntity_ == null)
        {
            playerEntity_ = ecsGroup.FindEntity(playerEntityName);
        }
        //見つからなければ false を返す
        return playerEntity_ != null;
    }

    private void CalcVelocityToPlayer()
    {
        // プレイヤーへのベクトルを計算
        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        if (toPlayer.Length() > 0.001f)
        {
            // velocityを計算
            velocity_ = toPlayer.Normalized() * chaseSpeed;
        }
    }

    private bool TickChaseTimer()
    {
        // 追いかけタイマーを更新
        chaseTimer_ += Time.deltaTime;

        // chaseDurationを超えたらWaitに遷移
        if (chaseTimer_ >= chaseDuration)
        {
            velocity_    = Vector3.zero;
            pulseTimer_  = 0.0f;
            waitTimer_   = 0.0f;
            isFirstWait_ = false;
            updateAction_ = Wait;
            return true;
        }
        return false;
    }
}
