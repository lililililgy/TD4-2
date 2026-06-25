using System;

// プレイヤーの移動ドライバ。物理は汎用 Mover に委譲し、ここは
// 「進みたい方向 = 入力」「移動パラメータ = 現在ステート」を Mover に渡すだけ。
public class PlayerMoveComponent : MonoScript {

    [SerializeField] private float paramReleaseSmoothTime_ = 0.25f;       // 減速側の追従時間（ダッシュの尾の長さ）
    [SerializeField] private float paramReleaseMaxSmoothSpeed_ = 100000.0f; // 追従の最大速度（実質クランプ無し）

    private Mover mover_;

    public override void Initialize() {
        mover_ = new Mover(paramReleaseSmoothTime_, paramReleaseMaxSmoothSpeed_);
    }

    public override void Update() {
        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();
        PlayerStateComponent stateComp = entity.GetScript<PlayerStateComponent>();

        Vector2 desiredDir = inputComp != null ? inputComp.MoveDir : new Vector2(0.0f, 0.0f);
        MoveParam param = stateComp != null ? stateComp.CurrentMoveParam() : null;

        mover_.Move(transform, desiredDir, param);
    }

    public Mover MoverData {
        get { return mover_; }
    }
}
