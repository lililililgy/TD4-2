using System;

// HP(体力)の管理だけに専念する汎用コンポーネント。
// ダメージ・回復・死亡判定・(死亡時の)自動破棄のみを扱う。持続回復などの味付けは持たない。
public class HP : MonoScript, IGaugeSource {
    [SerializeField] private float maxHp_ = 100;
    [SerializeField] private float currentHp_ = 0;
    // 開始時の HP。0 以上ならこの値から始まる（負なら maxHp_ から始まる＝従来動作）。
    // 「上限は高いが手持ちは少ない状態で始める」（例: プレイヤーの残機）を表現するためのもの。
    // 他スクリプトの Initialize から SetHp() で上書きする方式だと、
    // HP と書き込む側どちらの Initialize が先に走るかで結果が変わるため、ここで持つ。
    [SerializeField] private float initialHp_ = -1.0f;
    [SerializeField] private bool disableAutoDestruction_ = false; // true: 死亡しても自分で Destroy しない
    [SerializeField] private bool isInvincible_ = false; // true: 無敵状態（ダメージを受けない）
    [SerializeField] private bool directlyDamageable_ = true; // false: 直接被弾では削れない（中継=DamageRelay 経由のみ。共有ライフの本体/頭向け）

    private bool isDead_ = false;
    private float lastCurrentHp_ = 0;

    // TakeDamage() で実際に削れた量の累計。被弾演出はこの差分を自分の Update で取る。
    // 「今フレーム被弾した」フラグ方式にすると、フラグを落とすタイミングと
    // 読む側の実行順で取りこぼすため、累計値を公開して差分で判定させる。
    // ※ CurrentHp の減少では代用できない。プレイヤーの HP は残機(=卵数)と同期されており、
    //   発射で卵を消費したときにも減るため、被弾と区別が付かない。
    private float totalDamageTaken_ = 0.0f;

    public override void Initialize() {
        currentHp_ = initialHp_ >= 0.0f ? Mathf.Clamp(initialHp_, 0, maxHp_) : maxHp_;
        lastCurrentHp_ = currentHp_;
        isDead_ = false;
    }

    public override void Update() {
        lastCurrentHp_ = currentHp_;

        // 死亡時の自動破棄（衝突中のクラッシュ防止のため Update で破棄する）
        if (isDead_ && !disableAutoDestruction_) {
            // 撃破イベントを発行。Objective 系は MessageBus 経由でこれを購読してカウントする
            MessageBus.Publish(new EnemyKilledEvent(entity != null ? entity.name : ""));
            entity.Destroy();
        }
    }

    public void TakeDamage(float damage) {
        if (isDead_ || isInvincible_) return;

        float beforeHp = currentHp_;

        currentHp_ -= damage;
        if (currentHp_ <= 0) {
            currentHp_ = 0;
            isDead_ = true;
        }

        // オーバーキル分は数えず、実際に削れた量だけを積む
        totalDamageTaken_ += beforeHp - currentHp_;
    }

    public void Heal(int healAmount) {
        currentHp_ = Mathf.Clamp(currentHp_ + healAmount, 0, maxHp_);
        if (currentHp_ > 0) {
            isDead_ = false;
        }
    }

    // 現在HPを直接設定する（外部の真実と同期させたい時用。例: 残機=roe数 をHPへ反映）。
    public void SetHp(float hp) {
        currentHp_ = Mathf.Clamp(hp, 0, maxHp_);
        isDead_ = currentHp_ <= 0;
    }

    public bool HasHpChanged() {
        bool changed = currentHp_ != lastCurrentHp_;
        lastCurrentHp_ = currentHp_;
        return changed;
    }

    public float CurrentHpRatio() {
        return Mathf.Clamp01((float)currentHp_ / maxHp_);
    }

    // HPバー(SpriteGauge)が毎フレーム引く割合
    public float GetGaugeRatio() {
        return CurrentHpRatio();
    }

    public float MaxHp { get { return maxHp_; } set { maxHp_ = value; } }
    public float CurrentHp { get { return currentHp_; } }
    public bool IsDead { get { return isDead_; } }
    public bool IsInvincible { get { return isInvincible_; } set { isInvincible_ = value; } }
    public bool DisableAutoDestruction { get { return disableAutoDestruction_; } set { disableAutoDestruction_ = value; } }
    public bool IsDirectlyDamageable { get { return directlyDamageable_; } set { directlyDamageable_ = value; } }

    // TakeDamage() で削れた量の累計（単調増加）。前回値との差分で「被弾した量」を取る
    public float TotalDamageTaken { get { return totalDamageTaken_; } }
}
