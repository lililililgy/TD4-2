using System;

// Player の各状態の基底クラス（MonoScript）。
// 各 State は entity に付く独立スクリプトとして、自分の移動パラメーターを [SerializeField] で持つ。
// engine の Update() は使わず、PlayerStateComponent が現在 State にだけ OnStateUpdate を呼ぶ。
public abstract class PlayerState : MonoScript {

    // この状態の移動パラメーター（自分の SerializeField から構築する）
    public abstract PlayerMoveParam MoveParam { get; }

    // この状態で射撃が可能か
    public virtual bool CanShoot() {
        return true;
    }

    // 状態マシンから呼ばれるライフサイクル（engine の Update とは別物）
    public virtual void OnEnter() { }
    public virtual void OnStateUpdate() { }
    public virtual void OnExit() { }
}
