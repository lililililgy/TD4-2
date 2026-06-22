using System;
using System.Collections.Generic;

/// <summary>
/// 弾倉クラス。弾薬の実体は「幼生卵(LARVAE)」なので、残弾数は RoeManager から導出する。
/// 独立した残弾カウンタは持たず、リロード入力を受けて RoeManager に状態遷移を依頼するだけ。
/// リロード = 成熟卵(残機)を1つ犠牲にして幼生卵(弾薬)を1つ作る。
/// </summary>
public class Magazine : MonoScript {

    private RoeManager roeManager_;
    [SerializeField] private float reloadCooldown_ = 0.5f; // リロードのクールタイム（秒）
    private float reloadTimer_ = 0.0f; // リロードのクールタイムの残り時間（秒）

    public override void Initialize() {
        roeManager_ = entity.GetScript<RoeManager>();
        reloadTimer_ = reloadCooldown_;
    }

    public override void Update() {
        reloadTimer_ -= Time.deltaTime;
        reloadTimer_ = Math.Max(0.0f, reloadTimer_);

        PlayerInputComponent input = entity.GetScript<PlayerInputComponent>();
        if (input == null) {
            return;
        }

        // リロード（押した瞬間のみ）：成熟卵を1つ幼生卵に変える
        if (input.IsReloadButtonPressed && CanReload) {
            roeManager_.TryReload();
            reloadTimer_ = reloadCooldown_;
        }
    }

    // リロード可能 = 犠牲にできる成熟卵(残機)があり、弾(LARVAE)に空きがあり、クールタイムが明けている
    public bool CanReload {
        get {
            return roeManager_ != null
                && roeManager_.MatureCount() > 0
                && roeManager_.CanLoadAmmo
                && reloadTimer_ <= 0.0f;
        }
    }

    // 弾切れ = 幼生卵(弾薬)が無い
    public bool IsEmpty {
        get { return roeManager_ == null || roeManager_.LarveCount() <= 0; }
    }

    // 現在の残弾 = 幼生卵の数
    public int GetCurrentAmmo {
        get { return roeManager_ != null ? roeManager_.LarveCount() : 0; }
    }

    // 弾薬の上限 = 弾(LARVAE)の上限
    public int GetMaxAmmo {
        get { return roeManager_ != null ? roeManager_.MaxAmmo : 0; }
    }
}
