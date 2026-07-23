// 被弾したときの手触り演出をまとめて出す。HP と同じ Entity にアタッチする。
//   1. 一定時間の無敵(i-frame)。連続ヒットで一気に溶けるのを防ぐ。
//   2. コントローラーの振動(ラムブル)。
//   3. Time.timeScale を一定時間下げるヒットストップ。
//
// 検知は ShakeOnDamaged と同じく HP.TotalDamageTaken(累計)の差分で行う。理由:
//   ・CurrentHp の減少では代用できない。プレイヤーの HP は残機(卵数)と同期され、
//     発射で卵を消費したときにも減るため、被弾と区別が付かない。
//   ・「今フレーム被弾した」フラグ方式は、立てる側と読む側の実行順で取りこぼす。
//
// タイマーは Time.unscaledTime を基準にした締切方式にしている。ヒットストップ中は
// deltaTime も timeScale で縮むため、deltaTime を積むと無敵時間まで一緒に伸びてしまう。
// unscaledTime は実時間で進むので、狙った秒数で確実に解ける。
public class DamageFeedback : MonoScript {

    // --- 無敵 ---
    [SerializeField] private float invincibleDuration_ = 1.0f; // 被弾後この秒数だけ無敵(実時間)

    // --- 点滅 ---
    // 無敵中、スプライトの色を元色とこの色で交互に切り替えて点滅させる。
    // 既定は赤寄せ(被弾らしさ)。x=R, y=G, z=B, w=A。
    [SerializeField] private Vector4 blinkColor_    = new Vector4(1.0f, 0.4f, 0.4f, 1.0f);
    [SerializeField] private float   blinkInterval_ = 0.08f; // 1回の点灯/消灯の長さ(秒・実時間)

    // --- ヒットストップ ---
    [SerializeField] private float hitstopTimeScale_ = 0.15f; // この倍率まで一時的に減速
    [SerializeField] private float hitstopDuration_  = 0.12f; // 減速を保つ秒数(実時間)

    // --- 振動 ---
    // PlayGamepadVibration(左モーター, 右モーター, 継続秒) は継続時間をエンジン側で管理するので、
    // ここでタイマーを持つ必要はない。値の意味はエンジン依存なので調整用に露出しておく。
    [SerializeField] private float rumbleLeftMotor_  = 12.5f;
    [SerializeField] private float rumbleRightMotor_ = 0.8f;
    [SerializeField] private float rumbleDuration_   = 0.25f;

    private HP hp_;
    private float lastTotalDamage_;

    private SpriteRenderer renderer_;
    private Vector4 originalColor_;    // 点滅の戻り先(開始時のスプライト色)

    private bool  invincibleActive_ = false;
    private float invincibleUntil_  = 0.0f;

    private bool  hitstopActive_ = false;
    private float hitstopUntil_  = 0.0f;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        // 開始時点の累計を基準にする(初期化前の被弾で1発演出が出るのを防ぐ)
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;

        renderer_ = entity.GetComponent<SpriteRenderer>();
        if (renderer_ != null) {
            originalColor_ = renderer_.color; // 白とは限らないので実値を控える
        }
    }

    public override void Update() {
        if (hp_ == null) {
            return;
        }

        float now = Time.unscaledTime;

        float total = hp_.TotalDamageTaken;
        float damage = total - lastTotalDamage_;
        lastTotalDamage_ = total;

        // 実際に HP が削れたときだけ発火する。無敵中は TakeDamage が弾くので damage は増えず、
        // ここも自然に再発火しない(=既存の無敵を我々が誤って上書きすることもない)。
        if (damage > 0.0f) {
            TriggerFeedback(now);
        }

        // 無敵中の点滅。unscaledTime 基準なのでヒットストップ中も実時間で一定間隔に点滅する
        if (invincibleActive_) {
            UpdateBlink(now);
        }

        // 無敵の解除
        if (invincibleActive_ && now >= invincibleUntil_) {
            hp_.IsInvincible = false;
            invincibleActive_ = false;
            if (renderer_ != null) {
                renderer_.color = originalColor_; // 点滅を確実に元へ戻す
            }
        }

        // ヒットストップの解除(元の等速へ戻す)
        if (hitstopActive_ && now >= hitstopUntil_) {
            Time.timeScale = 1.0f;
            hitstopActive_ = false;
        }
    }

    private void TriggerFeedback(float now) {
        // 無敵
        hp_.IsInvincible = true;
        invincibleActive_ = true;
        invincibleUntil_  = now + invincibleDuration_;

        // ヒットストップ
        Time.timeScale = hitstopTimeScale_;
        hitstopActive_ = true;
        hitstopUntil_  = now + hitstopDuration_;

        // 振動
        Input.PlayGamepadVibration(rumbleLeftMotor_, rumbleRightMotor_, rumbleDuration_);
    }

    private void UpdateBlink(float now) {
        if (renderer_ == null) {
            return;
        }
        // interval が 0 以下だと割れるので、その場合は点滅させず元色のまま
        if (blinkInterval_ <= 0.0f) {
            renderer_.color = originalColor_;
            return;
        }
        // 経過を interval で刻み、偶奇で元色↔点滅色を交互に出す
        bool lit = ((int)(now / blinkInterval_) & 1) == 1;
        renderer_.color = lit ? blinkColor_ : originalColor_;
    }

    public override void OnDestroy() {
        // ヒットストップ中にプレイヤーが破棄されると Update が止まり、
        // timeScale が下がったままゲーム全体が減速する。破棄時に必ず戻す。
        if (hitstopActive_) {
            Time.timeScale = 1.0f;
            hitstopActive_ = false;
        }
    }
}
