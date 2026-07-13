using System;

// 「イベントを n 回数えたら達成」型の目標の generic 抽象基底。
// MessageBus の TEvent を購読してカウントする定型（購読・解除・リセット・進捗計算）をここに集約し、
// リーフは Accept() でフィルタ条件を注入するだけでよい。
// イベントは同期発行されるため、他スクリプトの実行順には依存しない。
//
// 注意: 閉じた generic を直接 entity.AddScript<T>() してはいけない
// （typeof(T).Name が「CountObjective`1」になり型名衝突するため）。
// generic はこの抽象基底に留め、具象リーフは必ず非 generic の薄いクラスにすること。
public abstract class CountObjective<TEvent> : Objective where TEvent : class {

    // 達成に必要なカウント数
    [SerializeField] protected int targetCount_ = 1;

    private int count_ = 0;
    private bool subscribed_ = false;

    // フェーズ開始時に ObjectiveSystem が呼ぶ。カウントをリセットする。
    // 購読は EndObjective() で解除されるため、ループでフェーズを再訪した際はここで張り直す。
    public override void BeginObjective() {
        count_ = 0;

        if (!subscribed_) {
            MessageBus.Subscribe<TEvent>(OnEvent);
            subscribed_ = true;
        }
    }

    // フェーズ離脱時の後始末。購読を解除する。
    public override void EndObjective() {
        UnsubscribeIfNeeded();
    }

    public override void OnDestroy() {
        UnsubscribeIfNeeded();
    }

    private void UnsubscribeIfNeeded() {
        if (subscribed_) {
            MessageBus.Unsubscribe<TEvent>(OnEvent);
            subscribed_ = false;
        }
    }

    private void OnEvent(TEvent e) {
        if (Accept(e)) {
            count_++;
        }
    }

    // リーフのフィルタ注入点。true を返したイベントだけカウントする。既定は全件カウント。
    protected virtual bool Accept(TEvent e) {
        return true;
    }

    public override bool IsCompleted() {
        // targetCount_ が 0 以下でも「1 回で達成」として防御する
        int target = targetCount_ > 0 ? targetCount_ : 1;
        return count_ >= target;
    }

    public override float Progress() {
        if (targetCount_ <= 0) return 1.0f;
        return Mathf.Clamp01((float)count_ / targetCount_);
    }

    // このフェーズ開始からのカウント数（UI 表示用）
    public int CurrentCount {
        get { return count_; }
    }

    public int TargetCount {
        get { return targetCount_; }
    }
}
