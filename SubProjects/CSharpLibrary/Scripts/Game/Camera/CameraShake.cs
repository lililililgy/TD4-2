using System;
using System.Collections.Generic;

// シェイクの計算方法
public enum ShakeSourceType {
    Noise,
}

// 同じ Entity に付いた ShakeSource を集めて合成し、カメラの transform へ反映する。
// カメラ用 Entity にアタッチする。
//
// 揺れの発生方法は3通り:
//   1. ShakeSource を同じ Entity に付けて、そちらの StartShake() を呼ぶ
//   2. CameraShake.Shake(duration, amplitude, frequency) でワンショットを流し込む
//   3. MessageBus に CameraShakeEvent を Publish する（攻撃のヒット演出などはこれ）
//
// 毎フレーム「前回加算した分を引いてから今回分を足す」ため、
// 他のスクリプト（追従・ズーム等）が同じ transform.position を触っていても実行順に依存しない。
public class CameraShake : MonoScript {

    // 全 ShakeSource にかかる倍率。0 にすると揺れを一括で切れる
    [SerializeField] private float shakeScale_ = 1.0f;

    // アタッチされている ShakeSource
    private List<ShakeSource> sources_ = new List<ShakeSource>();
    // Shake() で作った実行時専用の Source（Entity には付いていない）
    private List<ShakeSource> runtimeSources_ = new List<ShakeSource>();

    // 前フレームに transform へ加算したオフセット
    private Vector3 appliedOffset_ = Vector3.zero;
    private bool    hasAppliedOffset_;

    private float noiseSeedCounter_;

    // Unsubscribe に同じデリゲートを渡す必要があるので、生成したものを保持しておく
    private Action<CameraShakeEvent> shakeEventHandler_;

    public override void Awake() {
        shakeEventHandler_ = OnCameraShakeEvent;
        MessageBus.Subscribe(shakeEventHandler_);
    }

    public override void Initialize() {
        RefreshSources();
    }

    // アタッチされている ShakeSource を集め直す。実行時に追加した場合に呼ぶ
    public void RefreshSources() {
        sources_.Clear();
        if (entity == null) {
            return;
        }
        foreach (MonoScript script in entity.GetScripts()) {
            ShakeSource source = script as ShakeSource;
            if (source != null) {
                sources_.Add(source);
            }
        }
    }

    public override void Update() {
        // 前フレームの揺れを打ち消してから、今フレームの揺れを乗せ直す
        RemoveAppliedOffset();

        float deltaTime = Time.deltaTime;

        Vector3 total = Vector3.zero;
        total = total + Accumulate(sources_, deltaTime);
        total = total + Accumulate(runtimeSources_, deltaTime);

        // 終わったワンショットは溜め込まずに捨てる
        runtimeSources_.RemoveAll(source => !source.isActive);

        total = total * shakeScale_;
        if (total.x == 0.0f && total.y == 0.0f && total.z == 0.0f) {
            return;
        }

        appliedOffset_     = total;
        hasAppliedOffset_  = true;
        transform.position = transform.position + appliedOffset_;
    }

    public override void OnDestroy() {
        // 破棄後も配信が残らないよう、必ず購読を外す
        if (shakeEventHandler_ != null) {
            MessageBus.Unsubscribe(shakeEventHandler_);
            shakeEventHandler_ = null;
        }
        RemoveAppliedOffset();
    }

    private void OnCameraShakeEvent(CameraShakeEvent e) {
        Shake(e.duration, e.amplitude, e.frequency);
    }

    // その場限りの揺れを1回流す。ShakeSource を用意しない呼び出し向け
    public void Shake(float duration, float amplitude, float frequency) {
        if (duration <= 0.0f || amplitude <= 0.0f) {
            return;
        }

        ShakeSource source = new ShakeSource();
        source.Setup(
            ShakeSourceType.Noise,
            duration,
            amplitude,
            frequency > 0.0f ? frequency : 1.0f,
            noiseSeedCounter_);
        source.StartShake();
        runtimeSources_.Add(source);

        // ワンショットが重なったときに同じ揺れ方にならないよう、毎回パターンをずらす
        noiseSeedCounter_ += 13.1f;
    }

    // 実行中の揺れを全て止める
    public void StopAll() {
        foreach (ShakeSource source in sources_) {
            source.StopShake();
        }
        runtimeSources_.Clear();
    }

    private Vector3 Accumulate(List<ShakeSource> sources, float deltaTime) {
        Vector3 total = Vector3.zero;
        foreach (ShakeSource source in sources) {
            if (!source.enable) {
                continue;
            }
            // 持続時間を過ぎた Source はこのフレームぶんを加算しない
            if (!source.AdvanceTime(deltaTime)) {
                continue;
            }
            total = total + source.Evaluate();
        }
        return total;
    }

    private void RemoveAppliedOffset() {
        if (!hasAppliedOffset_) {
            return;
        }
        transform.position = transform.position - appliedOffset_;
        appliedOffset_    = Vector3.zero;
        hasAppliedOffset_ = false;
    }
}
