using System;

// ゲーム進行の「目標」の基底クラス（MonoScript）。
// 「n体倒せ」「ボスを倒せ」「チュートリアルの各操作」はすべてこの同じ抽象で表現し、
// ObjectiveSystem が常にアクティブな Objective 群を持ち、UI はここから進捗を読む想定。
// PlayerState と同様、entity に付く独立スクリプトとして自分のパラメーターを [SerializeField] で持つ。
public abstract class Objective : MonoScript {

    // UI 表示用の目標名。ゲーム内表示名はデータ側(SerializeField)に持たせる方針。
    [SerializeField] private string displayName_ = "";

    // フェーズ開始時に ObjectiveSystem が呼ぶ。引数なしに統一
    // （目標値などのパラメータは各 Objective 自身の [SerializeField] で完結させる）。
    public virtual void BeginObjective() { }

    // フェーズ離脱時に ObjectiveSystem が呼ぶ。イベント購読解除などの後始末用。
    public virtual void EndObjective() { }

    // 達成条件を満たしたか
    public abstract bool IsCompleted();

    // 進捗(0.0〜1.0)
    public abstract float Progress();

    // 進捗の表示文字列。UI はこれを呼ぶだけでよく、Objective の具象型を知る必要はない。
    // 既定はパーセント表示。「3 / 5」のように数値で見せたいものだけがオーバーライドする
    // （すべての目標が current/goal の整数ペアを持つわけではないため、
    //   数値を UI に取り出させるのではなく、表示のしかたを目標自身に答えさせる方針）。
    public virtual string ProgressText() {
        return (int)(Progress() * 100.0f) + "%";
    }

    public string DisplayName {
        get { return displayName_; }
    }
}
