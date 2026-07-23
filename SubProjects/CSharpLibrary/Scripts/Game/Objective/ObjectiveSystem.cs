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
public class ObjectiveSystem : MonoScript, IGaugeSource {

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
        JumpToPhase(ResolveStartIndex());
    }

    // 開始フェーズを決める。既定は先頭で、GameFlow.ResumeFromCheckPoint() 経由などで
    // フェーズ要求が出ている時だけそこから始める。
    // 要求が無い／要求されたフェーズ名が現在の列に見つからない場合（フェーズをリネーム・削除した、
    // セーブが古い等）も先頭に倒す。番号ではなく名前で引き直すのはこのため。
    private int ResolveStartIndex() {
        // 要求はワンショット。次に普通に GameScene を読んだ時は先頭から始まる。
        int index = IndexOfPhase(PhaseRequest.Consume());
        return index >= 0 ? index : 0;
    }

    public override void Update() {
        // プレイ中に出されたフェーズ要求（チュートリアルスキップ等）に追従する。
        // 進行終了後でも受け付けたいので finished_ の判定より先に引く。
        ConsumePhaseRequest();

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

    // 現在のフェーズの進捗(0.0〜1.0)。ProgressUI のゲージ(SpriteGauge)が毎フレーム引く。
    // 目標の具象型は見ずに Objective.Progress() だけを使うので、
    // 目標の種類が増えてもここは変わらない（ObjectiveProgressForText と同じ方針）。
    //
    // 1フェーズに複数の目標(AND条件)が積まれている場合は平均を取る。
    // 最小値ではなく平均なのは、片方だけ進んでいる間バーが一切動かないと
    // 進んでいないように見えるため。全部 1.0 にならないと 1.0 にならない点は変わらない。
    public float PhaseProgress {
        get {
            // 全フェーズ終了。バーは満タンのままにする（テキスト側は finishedText_ に切り替わる）
            if (finished_) return 1.0f;

            // フェーズエンティティの解決待ち。まだ目標が動き出していないので 0
            if (activeObjectives_.Count == 0) return 0.0f;

            float total = 0.0f;
            int count = 0;
            foreach (var objective in activeObjectives_) {
                if (objective == null) continue;

                total += Mathf.Clamp01(objective.Progress());
                count++;
            }

            return count > 0 ? total / count : 0.0f;
        }
    }

    // 進捗ゲージ(SpriteGauge)が毎フレーム引く割合
    public float GetGaugeRatio() {
        return PhaseProgress;
    }

    // 現在のフェーズエンティティ名。範囲外（未設定・進行終了など）なら ""
    public string CurrentPhaseName {
        get { return PhaseNameAt(currentIndex_); }
    }

    // フェーズ列の長さ
    public int PhaseCount {
        get { return phaseSequence_ != null ? phaseSequence_.Count : 0; }
    }

    // index 番目のフェーズ名。範囲外なら ""
    public string PhaseNameAt(int index) {
        if (phaseSequence_ == null || index < 0 || index >= phaseSequence_.Count) return "";
        return phaseSequence_[index];
    }

    // フェーズ列上の位置を名前で引く。見つからなければ -1。
    // 「この先どこまで飛ばすか」を外から決める側（PhaseSkipInput 等）がフェーズ列を
    // 直接持たずに済むよう、名前⇔位置の変換はここに集約する。
    public int IndexOfPhase(string phaseName) {
        if (phaseSequence_ == null || string.IsNullOrEmpty(phaseName)) return -1;
        return phaseSequence_.IndexOf(phaseName);
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

    // 名前でフェーズへ飛ぶ。列に無い名前なら何もせず false を返す。
    // 範囲外を進行終了に倒す JumpToPhase(int) と違い、こちらは知らない名前を無視する
    // （プレイ中の要求で、名前の打ち間違い1つがクリア扱いになるのを防ぐ）。
    public bool JumpToPhase(string phaseName) {
        int index = IndexOfPhase(phaseName);
        if (index < 0) return false;

        JumpToPhase(index);
        return true;
    }

    // 現在のフェーズを打ち切って次へ進める（達成を待たないスキップにも使える）。
    // 末尾を超えた index は JumpToPhase 側の範囲外処理で進行終了（finished_ = true）になる。
    public void AdvancePhase() {
        JumpToPhase(currentIndex_ + 1);
    }

    // 出ているフェーズ要求を1つ引いて、その名前のフェーズへ飛ぶ。
    // 要求が無ければ何もしない。飛んだ先で ContinueState.Save() が走るため、
    // スキップした結果もそのままコンティニュー地点になる。
    private void ConsumePhaseRequest() {
        string requestedPhaseName = PhaseRequest.Consume();
        if (string.IsNullOrEmpty(requestedPhaseName)) return;

        JumpToPhase(requestedPhaseName);
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

        // 突入したフェーズをコンティニュー地点として記録する。
        // ここは「フェーズが実際に始まった瞬間」なので、チェックポイントの定義そのものになる。
        ContinueState.Save(CurrentPhaseName);

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
