using System;

// チュートリアルフェーズのプレースホルダ。
// 個々の操作別の達成条件は未実装。今回は開始した時点で即達成扱いとし、
// ObjectiveSystem が次の Hunt フェーズへそのまま進められるようにする。
public class TutorialObjective : Objective {
    private bool completed_ = true;

    // ObjectiveSystem がチュートリアルフェーズ開始時に呼ぶ。
    public override void BeginObjective() {
        // TODO: 操作別の達成条件を実装する場合はここで completed_ = false にし、
        // 各操作の検知(移動/射撃/ダッシュ等)を追加する。
        completed_ = true;
    }

    public override bool IsCompleted() {
        return completed_;
    }

    public override float Progress() {
        return completed_ ? 1.0f : 0.0f;
    }
}
