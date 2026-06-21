using System;


public class SmallFryChaseMove : MonoScript {

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

    // 移動速度,追いかけタイマー
    private Vector3 velocity_ = Vector3.zero;
    private float chaseTimer_ = 0.0f;
    // 発見時のスケール、タイマー
    private float currentDiscoveryTimer_ = 0.0f;
    private Vector3 initialScale_ = new Vector3(1.0f, 1.0f, 1.0f);
    // entity
    private Entity playerEntity_ = null;
    // action
    private Action updateAction_ = null;

    public override void Initialize() {
        initialScale_ = transform.scale;
    }

    public override void Update() {
        // Action更新
        updateAction_?.Invoke();
    }

    public void StartChase() {
        // Playerエンティティを探す
        if (playerEntity_ == null) {
            playerEntity_ = ecsGroup.FindEntity(playerEntityName);
        }
        // チェイスActionに移行
        chaseTimer_ = 0.0f;
        updateAction_ = Wait;
    }

    private void ChaseMove() {

        // プレイヤーEntitynullチェック
        if (playerEntity_ == null) {
            return;
        }

        // 追いかけタイマー更新
        chaseTimer_ += Time.deltaTime;

        // 一定時間追いかけたらWaitに遷移する
        if (chaseTimer_ >= chaseDuration) {
            velocity_ = Vector3.zero;
            updateAction_ = Wait;
            return;
        }

        // プレイヤーへのベクトルを取得
        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        float distance = toPlayer.Length();

        // 近づきすぎたら方向固定で直進
        if (distance <= rushDistance) {
            updateAction_ = RushMove;
            return;
        }

        // プレイヤーの方向へ追尾する
        if (distance > 0.001f) {
            velocity_ = toPlayer.Normalized() * chaseSpeed;
        }

        // 位置適応・進行方向へ向く
        transform.position += velocity_ * Time.deltaTime;
        FaceVelocity();
    }

    private void RushMove() {
        transform.position += velocity_ * Time.deltaTime;
        FaceVelocity();
    }

    private void FaceVelocity() {
        // 動いていなかったら早期リターン
        if (velocity_.x == 0.0f && velocity_.y == 0.0f) {
            return;
        }
        // 進行方向へのターゲット回転を求めて、現在角度から Slerp で補間
        Quaternion targetRotate = Quaternion.LookRotation(-Vector3.forward, velocity_.Normalized());
        transform.rotate = Quaternion.Slerp(transform.rotate, targetRotate,0.3f);
    }

    private void Wait() {

        // プレイヤーEntityのnullチェック
        if (playerEntity_ == null) {
            return;
        }

        // 一定距離までプレイヤーがいたら発見モーションへ移行
        Vector3 toPlayer = playerEntity_.transform.position - transform.position;
        if (toPlayer.Length() <= chaseDistance) {
            currentDiscoveryTimer_ = 0.0f;
            updateAction_ = DiscoveryMotion;
        }
    }

    private void DiscoveryMotion() {
        // 発見演出タイマー更新
        currentDiscoveryTimer_ += Time.deltaTime;
        float halfTime = discoveryTimer * 0.5f;

        if (currentDiscoveryTimer_ < halfTime) {
            // 前半: EaseOutBack でスケールアップ
            float time = currentDiscoveryTimer_ / halfTime;
            float scale = 1.0f + (discoveryScale - 1.0f) * Ease.Out.Back(time);
            transform.scale = initialScale_ * scale;
        } else if (currentDiscoveryTimer_ < discoveryTimer) {
            // 後半: EaseOutQuad で元のスケールに戻す
            float time = (currentDiscoveryTimer_ - halfTime) / halfTime;
            float scale = discoveryScale + (1.0f - discoveryScale) * Ease.Out.Back(time);
            transform.scale = initialScale_ * scale;
        } else {
            // 演出終了 → ChaseMove へ
            transform.scale = initialScale_;
            chaseTimer_ = 0.0f;
            updateAction_ = ChaseMove;
        }
    }
}
