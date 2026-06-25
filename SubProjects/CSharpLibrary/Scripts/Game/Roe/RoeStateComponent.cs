using System;

public enum RoeState {
    UNMATURE, // 未成熟（成長中。残機にも弾薬にもならない）
    MATURE,   // 成熟（残機としてカウント）
    LARVAE    // 幼生（リロードで成熟卵を犠牲にした状態。弾薬としてカウント）
}

// 卵(roe)1個の状態。卵プレハブ側に付ける。状態が唯一の真実(single source of truth)で、
// 残機数・弾薬数は RoeManager がこの状態を数えて導出する。
//
// UNMATURE → MATURE は自分で成長する（本来は経験値、暫定で時間経過）。
// MATURE → LARVAE はリロード時に RoeManager から SetState で遷移させられる。
public class RoeStateComponent : MonoScript {
    [SerializeField] private float minScale_ = 32.0f;  // 幼生／成長前のスケール
    [SerializeField] private float maxScale_ = 64.0f;  // 成熟後のスケール

    private RoeState currentState_ = RoeState.UNMATURE;

    public override void Initialize() {
        currentState_ = RoeState.UNMATURE;
        ApplyScale(minScale_);
    }

    public override void Update() {
        // 成長するのは UNMATURE のときだけ。MATURE/LARVAE は遷移待ち。
        if (currentState_ != RoeState.UNMATURE) {
            return;
        }

        LevelingComponent levelingComponent = entity.GetScript<LevelingComponent>();
        ApplyScale(Mathf.Lerp(minScale_, maxScale_, levelingComponent.GetExpProgress()));

        // Level UP したら RoeState を MATURE に遷移させる。見た目もここで合わせる。
        if (levelingComponent.IsLevelUp) {
            SetState(RoeState.MATURE);
        }
    }

    public RoeState CurrentState { get { return currentState_; } }

    // 外部（RoeManager のリロード等）から状態を遷移させる。見た目もここで合わせる。
    public void SetState(RoeState next) {
        currentState_ = next;
        switch (next) {
            case RoeState.MATURE: ApplyScale(maxScale_); break;
            case RoeState.LARVAE: ApplyScale(minScale_); break; // 幼生に戻る
        }
    }

    private void ApplyScale(float s) {
        Transform transform = entity.GetComponent<Transform>();
        if (transform) {
            transform.scale = Vector3.one * s;
        }
    }
}
