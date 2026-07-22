using System;

// プレイヤーの移動ドライバ。役割を分離している：PlayerMoveComponent は「入力＋ステートの意図」を解釈して「最終的な進行方向」を求めるだけで、物理は Mover に任せる。
//   - MoveDirector : 入力＋ステートの意図(canMove/moveForward)を解釈し、最終的な進行方向を決める。
//   - Mover        : その方向を受け取り、物理（加速・旋回・最大速度・減速）だけを行う汎用クラス。
// ここは両者を繋ぎ、入力（PlayerInputComponent）とパラメータ（現在ステート）を供給するだけ。
public class PlayerMoveComponent : MonoScript
{

    [SerializeField] private float paramReleaseSmoothTime_ = 0.25f;       // 減速側の追従時間（ダッシュの尾の長さ）
    [SerializeField] private float paramReleaseMaxSmoothSpeed_ = 100000.0f; // 追従の最大速度（実質クランプ無し）

    private Mover mover_;
    private MoveDirector director_;
    private Rigidbody2D rigidbody_; // 速度の書き出し先。GetComponent は毎フレーム引かずここで持つ

    public override void Initialize()
    {
        mover_ = new Mover(paramReleaseSmoothTime_, paramReleaseMaxSmoothSpeed_);
        director_ = new MoveDirector();
        rigidbody_ = entity.GetComponent<Rigidbody2D>();
    }

    public override void Update()
    {
        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();
        PlayerStateComponent stateComp = entity.GetScript<PlayerStateComponent>();

        MoveParam param = stateComp != null ? stateComp.CurrentMoveParam() : null;
        if (param == null)
        {
            return;
        }

        // 入力を解釈して「最終的な進行方向」を求め、物理だけを Mover に任せる。
        Vector2 rawDir = inputComp != null ? inputComp.MoveDir : new Vector2(0.0f, 0.0f);
        Vector2 desiredDir = director_.Resolve(rawDir, param.canMove_, param.moveForward_);

        // 移動入力が「なし→あり」になったエッジで1回だけ発行する（チュートリアルの「移動した」検知用）。
        // 速度ではなく生の入力方向で判定する（ノックバック等の受動的な移動を誤検知しないため）。
        bool isMoveInputActive = rawDir.LengthSq() > 0.0001f;
        if (isMoveInputActive)
        {
            MessageBus.Publish(new PlayerMovingEvent());
        }

        if (rigidbody_ == null)
        {
            rigidbody_ = entity.GetComponent<Rigidbody2D>();
        }

        mover_.Move(transform, rigidbody_, desiredDir, param);
    }

    public Mover MoverData
    {
        get { return mover_; }
    }
}
