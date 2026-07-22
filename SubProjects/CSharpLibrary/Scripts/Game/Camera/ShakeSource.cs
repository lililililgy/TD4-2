using System;

// カメラシェイクの発生源。CameraShake と同じ Entity にアタッチする。
// 1つの Entity に複数付けてよく、CameraShake が全部を合成してカメラへ反映する。
// 自分では transform を触らないので、Update も持たない（時間送りは CameraShake が行う）。
public class ShakeSource : MonoScript {

    [SerializeField] private ShakeSourceType type_ = ShakeSourceType.Noise;
    [SerializeField] private bool  isActive_ = true;  // 起動時に揺れ始めるか
    [SerializeField] private bool  isLoop_   = true;  // true なら減衰せず StopShake() まで揺れ続ける
    [SerializeField] private float duration_ = 1.0f;  // 継続時間(秒)。ループ時は無視される

    // 軸ごとのパラメータ。amplitude は揺れ幅、frequency は揺れの速さ
    // 2D なので Z の amplitude は既定で 0（揺らしたいなら値を入れる）
    [SerializeField] private float amplitudeX_ = 1.0f;
    [SerializeField] private float frequencyX_ = 1.0f;
    [SerializeField] private float amplitudeY_ = 1.0f;
    [SerializeField] private float frequencyY_ = 1.0f;
    [SerializeField] private float amplitudeZ_ = 0.0f;
    [SerializeField] private float frequencyZ_ = 1.0f;

    // 値を変えると揺れのパターンが変わる。複数 Source を重ねるときは別々の値にする
    [SerializeField] private float noiseSeed_ = 0.0f;

    // 各軸で別のノイズ列を引くためのサンプル位置の差。近すぎると軸が揃って斜めにしか揺れない
    private const float kAxisSeparation = 37.7f;

    private float elapsedTime_;
    private float amplitudeScale_ = 1.0f;

    public bool isActive { get { return isActive_; } }

    public override void Initialize() {
        // C++ 版と同じく、有効な状態で置かれていたらそのまま揺れ始める
        if (isActive_) {
            StartShake();
        }
    }

    // シェイクを開始する。経過時間をリセットする
    public void StartShake() {
        StartShake(1.0f);
    }

    // amplitudeScale で軸ごとの amplitude をまとめて倍率調整して開始する
    public void StartShake(float amplitudeScale) {
        amplitudeScale_ = amplitudeScale;
        elapsedTime_    = 0.0f;
        isActive_       = true;
    }

    // シェイクを停止する（非アクティブ化）
    public void StopShake() {
        isActive_ = false;
    }

    // 実行時にパラメータごと差し替える。CameraShake.Shake() のワンショット用
    public void Setup(ShakeSourceType type, float duration, float amplitude, float frequency, float noiseSeed) {
        type_       = type;
        isLoop_     = false;
        duration_   = duration;
        amplitudeX_ = amplitude;
        amplitudeY_ = amplitude;
        amplitudeZ_ = 0.0f;
        frequencyX_ = frequency;
        frequencyY_ = frequency;
        frequencyZ_ = frequency;
        noiseSeed_  = noiseSeed;
    }

    // 時間を進める。持続時間を超えたら非アクティブ化して false を返す
    public bool AdvanceTime(float deltaTime) {
        if (!isActive_) {
            return false;
        }

        elapsedTime_ += deltaTime;

        if (!isLoop_ && duration_ <= elapsedTime_) {
            isActive_ = false;
            return false;
        }
        return true;
    }

    // 現在の揺れ量を返す
    public Vector3 Evaluate() {
        switch (type_) {
        case ShakeSourceType.Noise:
            return EvaluateNoise();
        default:
            return Vector3.zero;
        }
    }

    private Vector3 EvaluateNoise() {
        // ループ中は減衰させず、それ以外は残り時間に比例して収束させる
        float damping = 1.0f;
        if (!isLoop_ && duration_ > 0.0f) {
            damping = Mathf.Clamp01(1.0f - elapsedTime_ / duration_);
        }
        float scale = amplitudeScale_ * damping;

        // 時間軸に沿ってノイズを走査する。軸ごとに離れた行を引いて相関を切る
        float x = FbmNoise.Fbm11(new Vector2(elapsedTime_ * frequencyX_, noiseSeed_));
        float y = FbmNoise.Fbm11(new Vector2(elapsedTime_ * frequencyY_, noiseSeed_ + kAxisSeparation));
        float z = FbmNoise.Fbm11(new Vector2(elapsedTime_ * frequencyZ_, noiseSeed_ + kAxisSeparation * 2.0f));

        return new Vector3(
            x * amplitudeX_ * scale,
            y * amplitudeY_ * scale,
            z * amplitudeZ_ * scale);
    }
}
