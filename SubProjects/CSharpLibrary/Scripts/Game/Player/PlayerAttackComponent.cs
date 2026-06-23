using System;
using System.Collections.Generic;

/// <summary>
/// プレイヤーの攻撃。頭での衝突ダメージを、移動速度に応じて ResponseCurve で決め、
/// 毎フレーム AttackCollision.Damage へ反映する。
/// 速度→ダメージの算出はこのクラスの責務で、AttackCollision は与えられた Damage を使うだけ。
/// </summary>
public class PlayerAttackComponent : MonoScript {

    // 速度→ダメージ の折れ線。1要素＝グラフ上の1点（speed のとき damage）。
    // 範囲外は端の値でクランプ。speed の昇順は評価直前に自前で保証するので並び順は自由。
    [SerializeField] private List<DamagePoint> damageCurve_ = new List<DamagePoint> {
        new DamagePoint { speed = 0.0f,    damage = 0.0f  },
        new DamagePoint { speed = 300.0f,  damage = 10.0f },
        new DamagePoint { speed = 600.0f,  damage = 30.0f },
        new DamagePoint { speed = 1000.0f, damage = 50.0f },
    };

    private AttackCollision atkColl_;
    private PlayerMoveComponent moveComp_;

    public override void Initialize() {
        atkColl_  = entity.GetScript<AttackCollision>();
        moveComp_ = entity.GetScript<PlayerMoveComponent>();
    }

    public override void Update() {
        if (atkColl_ == null || moveComp_ == null || moveComp_.MoverData == null) {
            return;
        }

        // ランタイム（Editor）で順序が崩れても、評価直前に必ず speed 昇順へ整える。
        damageCurve_?.Sort((a, b) => a.speed.CompareTo(b.speed));

        float speed = moveComp_.MoverData.Velocity.Length();
        atkColl_.Damage = ResponseCurve.Evaluate(damageCurve_, speed);
    }
}
