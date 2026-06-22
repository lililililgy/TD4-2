using System;

// プレイヤー（母体）の残機（Life）を管理する。
// 残機の実体は「成熟した卵(roe)」なので、数は RoeManager から取得する。
// このコンポーネントは Life の意味づけ（残機数・ゲームオーバー判定）に徹し、卵そのものは扱わない。
public class PlayerLifeComponent : MonoScript {

    private RoeManager roeManager_;
    private bool isAlive_ = true;

    public override void Initialize() {
        roeManager_ = entity.GetScript<RoeManager>();
        isAlive_ = true;
    }

    public override void OnCollisionEnter(Entity collision) {
        if (RemainingLives() <= 0) {
            isAlive_ = false;
        }
    }

    // 現在の残機数（＝成長後＝成熟した卵の数）
    public int RemainingLives() {
        return roeManager_ != null ? roeManager_.MatureCount() : 0;
    }

    // 残機が尽きたか（ゲームオーバー判定）
    public bool IsGameOver() {
        return !isAlive_;
    }
}
