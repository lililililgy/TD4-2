using System;

// HP(体力)の管理だけに専念する汎用コンポーネント。
// ダメージ・回復・死亡判定・(死亡時の)自動破棄のみを扱う。持続回復などの味付けは持たない。
public class HP : MonoScript {
    [SerializeField] private float maxHp_ = 100;
    [SerializeField] private float currentHp_ = 0;
    [SerializeField] private bool disableAutoDestruction_ = false; // true: 死亡しても自分で Destroy しない
    [SerializeField] private bool isInvincible_ = false; // true: 無敵状態（ダメージを受けない）
    [SerializeField] private bool directlyDamageable_ = true; // false: 直接被弾では削れない（中継=DamageRelay 経由のみ。共有ライフの本体/頭向け）

    private bool isDead_ = false;
    private float lastCurrentHp_ = 0;

    public override void Initialize() {
        currentHp_ = maxHp_;
        lastCurrentHp_ = currentHp_;
        isDead_ = false;
    }

    public override void Update() {
        lastCurrentHp_ = currentHp_;

        // 死亡時の自動破棄（衝突中のクラッシュ防止のため Update で破棄する）
        if (isDead_ && !disableAutoDestruction_) {
            entity.Destroy();
        }
    }

    public void TakeDamage(float damage) {
        if (isDead_ || isInvincible_) return;

        currentHp_ -= damage;
        if (currentHp_ <= 0) {
            currentHp_ = 0;
            isDead_ = true;
        }
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

    public float MaxHp { get { return maxHp_; } set { maxHp_ = value; } }
    public float CurrentHp { get { return currentHp_; } }
    public bool IsDead { get { return isDead_; } }
    public bool IsInvincible { get { return isInvincible_; } set { isInvincible_ = value; } }
    public bool DisableAutoDestruction { get { return disableAutoDestruction_; } set { disableAutoDestruction_ = value; } }
    public bool IsDirectlyDamageable { get { return directlyDamageable_; } set { directlyDamageable_ = value; } }
}
