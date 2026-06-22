using System;

// 状態遷移1本ぶんの定義（どの状態から・どの状態へ・どの条件で）。
// MonoScript ではなく PlayerStateComponent の遷移テーブルが保持する。
public class PlayerStateTransition {
    public PlayerState From { get; private set; }
    public PlayerState To { get; private set; }
    public Func<bool> Condition { get; private set; }

    public PlayerStateTransition(PlayerState from, PlayerState to, Func<bool> condition) {
        From = from;
        To = to;
        Condition = condition;
    }
}
