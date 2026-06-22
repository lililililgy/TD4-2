using System;
using System.Collections.Generic;

// Player の状態管理 (MonoScript)。各 State は entity 上の独立スクリプト。
// このクラスは State 参照の取得・遷移グラフ・遷移評価・現在 State の OnStateUpdate 呼び出しに専念する。
public class PlayerStateComponent : MonoScript {

    private PlayerNormalState normalState_;
    private PlayerDashState   dashState_;

    private readonly List<PlayerStateTransition> transitions_ = new List<PlayerStateTransition>();
    private PlayerState currentState_;

    // 現在の状態名（インスペクタ確認用）。再生中に現在 State が表示される。
    [SerializeField] private string currentStateName_ = "";

    // State が取得できないときの保険（移動が壊れないように既定値を返す）
    private readonly PlayerMoveParam fallbackMoveParam_ = new PlayerMoveParam();


    public override void Initialize() {
        normalState_ = entity.GetScript<PlayerNormalState>();
        dashState_   = entity.GetScript<PlayerDashState>();

        BuildTransitions();

        currentState_ = normalState_;
        if (currentState_ != null) {
            currentStateName_ = currentState_.GetType().Name;
            currentState_.OnEnter();
        }
    }

    public override void Update() {
        if (currentState_ == null) {
            return;
        }

        // 遷移評価（中央1箇所・宣言順＝優先順位・現在状態に宣言された遷移先のみ）
        foreach (PlayerStateTransition t in transitions_) {
            if (ReferenceEquals(t.From, currentState_) && t.Condition()) {
                ChangeState(t.To);
                break;
            }
        }

        currentState_.OnStateUpdate();
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    /// 遷移テーブル（どの状態からどこへ。条件の中身は各 State に委譲する）
    ///////////////////////////////////////////////////////////////////////////////////////////
    private void BuildTransitions() {
        if (normalState_ == null || dashState_ == null) {
            return;
        }
        //            From          To            条件（具体ロジックは State 側）
        AddTransition(normalState_, dashState_,   () => dashState_.WantsToStart());
        AddTransition(dashState_,   normalState_, () => dashState_.IsFinished());
    }

    private void AddTransition(PlayerState from, PlayerState to, Func<bool> condition) {
        transitions_.Add(new PlayerStateTransition(from, to, condition));
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    /// 状態遷移の実行
    ///////////////////////////////////////////////////////////////////////////////////////////
    private void ChangeState(PlayerState next) {
        if (next == null || ReferenceEquals(next, currentState_)) {
            return;
        }
        currentState_.OnExit();
        currentState_ = next;
        currentStateName_ = currentState_.GetType().Name;
        currentState_.OnEnter();
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    /// 現在状態の能力を外へ公開（判断の中身は各 State が持つ。ここは委譲のみ）
    ///////////////////////////////////////////////////////////////////////////////////////////
    public PlayerMoveParam CurrentMoveParam() {
        if (currentState_ == null || currentState_.MoveParam == null) {
            return fallbackMoveParam_;
        }
        return currentState_.MoveParam;
    }

    public bool CanShoot() {
        return currentState_ == null ? true : currentState_.CanShoot();
    }
}
