using System;

// プレイヤー（母体）の残機(Life)と HP を橋渡しする。
// 残機 = HP = 全卵数（未成熟の子たまごも含む）。発射でも被弾でも卵が減れば HP も減る。
//   HP.CurrentHp == 全roe数(残機)、HP.MaxHp == 卵の上限(MaxRoe)。
//
// 汎用 AttackCollision → HP.TakeDamage で減った分を、このブリッジが成熟roeの破棄に変換する。
// HP 自体は roe を知らない（HP 管理に専念）。roe を真実とし、毎フレーム HP を同期する。
public class PlayerLifeComponent : MonoScript {

    private RoeManager roeManager_;
    private HP hp_;
    private bool isAlive_ = true;
    private bool hadLife_ = false;     // 一度でも残機を持ったか（産卵前の開始直後に即ゲームオーバーしない保険）
    private float lastSyncedHp_ = 0.0f; // 前フレームに roe へ同期した HP。被ダメージ検出の基準。

    public override void Initialize() {
        roeManager_ = entity.GetScript<RoeManager>();
        hp_ = entity.GetScript<HP>();
        isAlive_ = true;
        hadLife_ = false;

        if (hp_ != null) {
            hp_.DisableAutoDestruction = true; // 死亡判定はこのブリッジが担う（HPは自分で破棄しない）
            if (roeManager_ != null) {
                hp_.MaxHp = roeManager_.MaxRoe;
                hp_.SetHp(roeManager_.EggCount());
            }
            lastSyncedHp_ = hp_.CurrentHp;
        }
    }

    public override void Update() {
        if (hp_ == null || roeManager_ == null) {
            return;
        }

        hp_.MaxHp = roeManager_.MaxRoe;

        // 被ダメージ検出は「前回同期した HP より減ったか」で行う。
        // ※ 現在の残機(lives)と比較すると、roe が成熟して残機が増えた時にも成立してしまい、
        //   成熟した roe を即破棄してしまう（spawn 直後に消えるバグの原因）。
        int damage = (int)(lastSyncedHp_ - hp_.CurrentHp);
        for (int i = 0; i < damage; i++) {
            if (!roeManager_.TryKillLife()) {
                break;
            }
        }

        // roe(残機) に HP を合わせる（産卵で増えた／発射・被弾で減った、いずれもここで同期）
        int lives = roeManager_.EggCount();
        hp_.SetHp(lives);
        lastSyncedHp_ = hp_.CurrentHp;

        if (lives > 0) {
            hadLife_ = true;
        }

        // 生存→死亡のエッジで1回だけ発行する（残機0の間ずっと流し続けない）
        bool wasAlive = isAlive_;
        isAlive_ = !hadLife_ || lives > 0;
        if (wasAlive && !isAlive_) {
            MessageBus.Publish(new PlayerDeadEvent());
        }
    }

    // 現在の残機数（＝全卵数。未成熟の子たまごも含む）
    public int RemainingLives() {
        return roeManager_ != null ? roeManager_.EggCount() : 0;
    }

    // 残機が尽きたか（ゲームオーバー判定）
    public bool IsGameOver() {
        return !isAlive_;
    }
}
