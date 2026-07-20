using System;
using System.Collections.Generic;

// 目標(Objective)システムの進行管理。「1フェーズ = 1エンティティ」方式。
// phaseSequence_ にはフェーズエンティティの名前を順に並べる
// （例: Phase_Tutorial → Phase_Hunt → Phase_Boss → …）。
// 各フェーズエンティティには Objective 派生スクリプト（KillCountObjective 等）を積み、
// 目標値などのパラメータは各 Objective 自身の [SerializeField] で設定する。
// 1つのフェーズエンティティに複数の Objective を積めば AND 条件（全達成でフェーズ完了）になる。
// 末尾のフェーズを終えると進行終了（IsFinished が true）になる。
//
// フェーズ突入時はエンティティ名を ecsGroup.FindEntity で解決し、
// GetScripts() から Objective を収集して BeginObjective() を呼ぶ。
// フェーズエンティティがまだ生成されていなければ「解決待ち」となり、Update() で毎フレーム再試行する
// （解決できるまで目標は空 = 完了扱いにはならないので、進行が誤って進むことはない）。
//
// 常に「今アクティブな Objective」を保持し、UI はここを見て表示する想定。
// シーン遷移そのものは GameSceneController の責務。ここでは目標達成判定とフェーズ送りだけを行う。
public class ObjectiveSystem : MonoScript {

    // フェーズ列（フェーズエンティティ名の並び）。ここを書き換えるだけで進行順を自由に変えられる。
    // チュートリアルを飛ばしたい場合は Phase_Tutorial の項目を消せばよい。
    [SerializeField] private List<string> phaseSequence_ = new List<string> {
        "Phase_Tutorial",
        "Phase_Hunt",
        "Phase_Boss",
    };

    private int currentIndex_ = 0;
    private bool finished_ = false;
    // フェーズエンティティを解決済みか。false の間は Update() で毎フレーム再試行する
    private bool phaseEntityResolved_ = false;
    // PhaseBeganEvent を発行済みか。PhaseEndedEvent を必ず対で出すために持つ
    private bool phaseBegan_ = false;
    private List<Objective> activeObjectives_ = new List<Objective>();

    public override void Initialize() {
        JumpToPhase(0);
    }

    public override void Update() {
        if (finished_) return;

        // フェーズエンティティが未生成だった場合の遅延解決
        if (!phaseEntityResolved_) {
            TryResolvePhaseEntity();
            if (!phaseEntityResolved_) return;
        }

        if (IsPhaseCompleted()) {
            AdvancePhase();
        }
    }

    // 現在アクティブな目標一覧（UI 表示用）
    public List<Objective> ActiveObjectives {
        get { return activeObjectives_; }
    }

    // 現在のフェーズエンティティ名。範囲外（未設定・進行終了など）なら ""
    public string CurrentPhaseName {
        get {
            if (phaseSequence_ == null ||
                currentIndex_ < 0 || currentIndex_ >= phaseSequence_.Count) {
                return "";
            }
            return phaseSequence_[currentIndex_];
        }
    }

    // 現在のフェーズ列上の位置（0始まり）
    public int CurrentPhaseIndex {
        get { return currentIndex_; }
    }

    // フェーズ列を最後まで終えた（末尾のフェーズを完了した）か
    public bool IsFinished {
        get { return finished_; }
    }

    // 現在のフェーズが開始済み（PhaseBeganEvent 発行済み）か。
    // PhaseBeganEvent を取りこぼしうる別 ECSGroup の購読者が、
    // イベントの代わりに毎フレーム引くための状態。
    public bool IsPhaseBegan {
        get { return phaseBegan_; }
    }

    // 任意のフェーズへ飛ぶ（デバッグ・イベント演出・外部スクリプトからの制御用）。
    // 範囲外の index を渡すと進行終了扱いになる。
    // 解決待ち中に呼ばれた場合は待ちを破棄して新しい対象に切り替える。
    public void JumpToPhase(int index) {
        // 前フェーズの目標に後始末をさせてから手放す
        EndActiveObjectives();

        if (phaseSequence_ == null || phaseSequence_.Count == 0 ||
            index < 0 || index >= phaseSequence_.Count) {
            finished_ = true;
            phaseEntityResolved_ = true;
            return;
        }

        finished_ = false;
        currentIndex_ = index;
        phaseEntityResolved_ = false;
        TryResolvePhaseEntity();
    }

    // 現在のフェーズを打ち切って次へ進める（達成を待たないスキップにも使える）。
    // 末尾を超えた index は JumpToPhase 側の範囲外処理で進行終了（finished_ = true）になる。
    public void AdvancePhase() {
        JumpToPhase(currentIndex_ + 1);
    }

    private bool IsPhaseCompleted() {
        if (activeObjectives_.Count == 0) return false;

        foreach (var objective in activeObjectives_) {
            if (objective == null) continue;
            if (!objective.IsCompleted()) return false;
        }
        return true;
    }

    // 現在のフェーズエンティティの解決を試みる。
    // 見つかったら Objective スクリプトを収集して BeginObjective() を呼び、アクティブ化する。
    private void TryResolvePhaseEntity() {
        Entity phaseEntity = ecsGroup.FindEntity(phaseSequence_[currentIndex_]);
        if (phaseEntity == null) return; // 未生成。解決待ちを継続する

        activeObjectives_.Clear();
        foreach (MonoScript script in phaseEntity.GetScripts()) {
            Objective objective = script as Objective;
            if (objective == null) continue;

            objective.BeginObjective();
            activeObjectives_.Add(objective);
        }
        phaseEntityResolved_ = true;

        // 目標の BeginObjective() を済ませてから通知する。
        // これで購読側は「目標が動き出した状態」を前提にしてよい。
        phaseBegan_ = true;
        MessageBus.Publish(new PhaseBeganEvent(CurrentPhaseName));
    }

    // アクティブな目標すべてに EndObjective()（購読解除などの後始末）を呼んでクリアする
    private void EndActiveObjectives() {
        foreach (var objective in activeObjectives_) {
            if (objective == null) continue;
            objective.EndObjective();
        }
        activeObjectives_.Clear();

        // PhaseBeganEvent を出したフェーズにだけ、対になる終了を通知する
        // （Initialize() の JumpToPhase(0) でもここを通るため、フラグで空撃ちを防ぐ）。
        // currentIndex_ を進める前に呼ばれるので、CurrentPhaseName はまだ「終わるフェーズ」を指す。
        if (phaseBegan_) {
            phaseBegan_ = false;
            MessageBus.Publish(new PhaseEndedEvent(CurrentPhaseName));
        }
    }
}
