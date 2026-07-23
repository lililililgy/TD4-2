// ヒットストップ(Time.timeScale の一時的な減速)の唯一の持ち主。
// 「減速してほしい」側は HitStop.Request() を呼ぶだけで、timeScale を直接触らない。
//
// timeScale はゲーム全体で1つしかない共有資源なので、複数のスクリプトがそれぞれタイマーを持って
// 書き戻すと、片方の解除がもう片方の減速を打ち消してしまう（例: 攻撃ヒットと被弾が同フレームに
// 起きた場合）。要求を1箇所に集約し、重なった要求は「強い方・長い方」で1本にまとめる。
//
// 時間を進めるのは HitStopSystem（MonoScript）の役目。こちらは状態と合成規則だけを持つ。
// MonoScript ではないのでアタッチは不要。
//
// タイマーは Time.unscaledDeltaTime(実フレーム差分)を積算して測る。減速中は deltaTime が
// timeScale の分だけ縮むため、deltaTime を積むと解除までの実時間が伸び続けてしまう。
public static class HitStop {

    private static float remaining_ = 0.0f; // 解除までの残り時間(実時間・秒)
    private static float scale_     = 1.0f; // 現在かけている減速率
    private static bool  active_    = false;

    // 時間を進める担当（HitStopSystem）が居るか。居ないシーンで要求だけが通ると、
    // 誰も解除しないままゲーム全体が減速し続ける事故になるため、その場合は要求を捨てる。
    private static bool hasDriver_ = false;

    // 減速を要求する。timeScale は 0〜1（小さいほど強い）、duration は実時間の秒数。
    // 既に減速中なら、率は強い方(小さい方)・残り時間は長い方を採用する。
    // 弱い要求が強い要求を上書きして演出を殺すことがなく、呼び出し順にも依存しない。
    public static void Request(float timeScale, float duration) {
        if (!hasDriver_ || duration <= 0.0f) {
            return;
        }

        timeScale = Mathf.Clamp(timeScale, 0.0f, 1.0f);
        if (timeScale >= 1.0f) {
            return; // 減速にならない要求は無視
        }

        if (!active_ || timeScale < scale_) {
            scale_ = timeScale;
        }
        if (!active_ || duration > remaining_) {
            remaining_ = duration;
        }
        active_    = true;

        Time.timeScale = scale_;
    }

    // HitStopSystem から毎フレーム呼ばれる。unscaledDt は実フレーム差分(秒)。
    public static void Tick(float unscaledDt) {
        if (!active_) {
            return;
        }

        remaining_ -= unscaledDt;
        if (remaining_ > 0.0f) {
            // 他所が timeScale を書き換えていても、減速中は毎フレーム自分の値へ引き戻す
            Time.timeScale = scale_;
            return;
        }

        Cancel();
    }

    // 即座に等速へ戻す。シーン遷移や HitStopSystem の破棄時に必ず通す。
    public static void Cancel() {
        remaining_ = 0.0f;
        scale_     = 1.0f;
        if (active_) {
            active_ = false;
            Time.timeScale = 1.0f;
        }
    }

    // HitStopSystem だけが呼ぶ。担当が居ない間の要求は Request() 側で捨てられる。
    public static void SetDriverPresent(bool present) {
        hasDriver_ = present;
        if (!present) {
            Cancel(); // 担当が消えたら減速を残さない
        }
    }

    public static bool IsActive   { get { return active_; } }
    public static bool HasDriver  { get { return hasDriver_; } }
}
