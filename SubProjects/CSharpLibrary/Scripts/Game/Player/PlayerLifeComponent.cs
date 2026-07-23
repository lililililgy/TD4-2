using System;

// プレイヤー（母体）の残機(Life)を司る。残機の真実は HP で、卵(roe)はそれに追従する。
//   HP.CurrentHp == 残機 == 卵数、HP.MaxHp == 卵の上限。開始時の残機は HP の initialHp_。
//
// 被弾(AttackCollision → HP.TakeDamage)も発射(RoeManager.TryConsumeMature)も HP を減らし、
// 減った分の卵を RoeManager が消す。このクラスは HP を卵へ同期する仕事を持たない。
//
// このクラスの責務は「レベルアップぶんの残機回復」と「ゲームオーバー判定」の2つだけ。
public class PlayerLifeComponent : MonoScript {

    // レベルが1つ上がるごとに回復する残機。HP.MaxHp を超えた分は切り捨てられる
    [SerializeField] private int lifePerLevelUp_ = 1;

    private HP hp_;
    private LevelingComponent leveling_;
    private bool isAlive_ = true;
    private bool hadLife_ = false;  // 一度でも残機を持ったか（開始直後に即ゲームオーバーしない保険）
    private int lastLevel_ = 0;     // 前フレームまでに回復に反映済みのレベル

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        leveling_ = entity.GetScript<LevelingComponent>();
        isAlive_ = true;
        hadLife_ = false;

        if (hp_ != null) {
            hp_.DisableAutoDestruction = true; // 死亡判定はこのクラスが担う（HPは自分で破棄しない）
        }
        lastLevel_ = leveling_ != null ? leveling_.CurrentLevel : 0;
    }

    public override void Update() {
        if (hp_ == null) {
            return;
        }

        // レベルアップぶん残機を回復する。
        // ※ LevelingComponent.IsLevelUp は毎フレーム先頭でリセットされるフラグなので、
        //   スクリプト実行順によっては取りこぼす。累計値であるレベルの差分で取る。
        if (leveling_ != null) {
            int level = leveling_.CurrentLevel;
            int gainedLevels = level - lastLevel_;
            lastLevel_ = level;

            if (gainedLevels > 0 && lifePerLevelUp_ > 0) {
                hp_.Heal(gainedLevels * lifePerLevelUp_);
            }
        }

        int lives = (int)hp_.CurrentHp;
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

    // 現在の残機数（＝HP。卵の数もこれに追従する）
    public int RemainingLives() {
        return hp_ != null ? (int)hp_.CurrentHp : 0;
    }

    // 残機が尽きたか（ゲームオーバー判定）
    public bool IsGameOver() {
        return !isAlive_;
    }
}
