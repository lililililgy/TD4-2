using System;

// 卵(roe)の成長段階。卵プレハブ側に付ける。
// 生まれたては「残機としてカウントされない」状態で始まり、成長すると「カウントされる」状態になる。
// 本来は経験値で成長するが、経験値が未実装のため、現状は時間経過で成長する。
public class RoeGrowth : MonoScript {

    [SerializeField] private float growTime_ = 5.0f;   // この秒数で残機としてカウントされる（暫定：時間成長）

    [SerializeField] private float minScale_ = 32.0f;    // 成長前の最小スケール
    [SerializeField] private float maxScale_ = 64.0f;   // 成長後の最大スケール

    private float currentTimer_ = 0.0f;
    private bool isMature_ = false;

    public override void Initialize() {
        currentTimer_ = 0.0f;
        isMature_ = false;
    }

    public override void Update() {
        if (isMature_) {
            return;
        }

        currentTimer_ += Time.deltaTime;

        Transform transform = entity.GetComponent<Transform>();
        if (transform) {
            transform.scale = Vector3.one * Mathf.Lerp(minScale_, maxScale_, currentTimer_ / growTime_);
        }

        // TODO: 経験値が実装されたら、この条件を「経験値 >= 必要量」に差し替える。
        if (currentTimer_ >= growTime_) {
            isMature_ = true;
        }
    }

    // 残機としてカウントされる状態か（成熟したか）
    public bool IsMature() {
        return isMature_;
    }
}
