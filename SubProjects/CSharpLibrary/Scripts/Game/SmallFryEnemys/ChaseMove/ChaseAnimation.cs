using System;

// ChaseController にアタッチするスケールアニメーション
public class ChaseAnimation : MonoScript
{
    // 発見演出
    [SerializeField] private float discoveryScale    = 2.0f;
    [SerializeField] private float discoveryDuration = 0.5f;
    // Wait 中パルス
    [SerializeField] private float waitPulseScale    = 1.2f;
    [SerializeField] private float waitPulseDuration = 1.0f;
    // チェイス中パルス
    [SerializeField] private float chasePulseScale    = 1.4f;
    [SerializeField] private float chasePulseDuration = 0.4f;

    private ChaseController chase_;
    private Vector3         initialScale_;
    private float           animTimer_  = 0.0f;
    private float           pulseTimer_ = 0.0f;

    public override void Initialize()
    {
        // ChaseController 取得・初期スケール保存
        chase_        = entity.GetScript<ChaseController>();
        initialScale_ = transform.scale;

        // 発見演出の長さを ChaseController に伝える
        chase_.DiscoveryDuration = discoveryDuration;

        // 状態遷移時にタイマーをリセット
        chase_.OnWaitStart      += () => { pulseTimer_ = 0.0f; transform.scale = initialScale_; };
        chase_.OnDiscoveryStart += () => { animTimer_  = 0.0f; transform.scale = initialScale_; };
        chase_.OnChaseStart     += () => { pulseTimer_ = 0.0f; };
        chase_.OnRushStart      += () => { pulseTimer_ = 0.0f; };
    }

    public override void Update()
    {
        // 状態に応じたアニメーションを再生
        switch (chase_.CurrentState)
        {
            case ChaseController.State.Wait:
                UpdatePulse(waitPulseScale, waitPulseDuration);
                break;
            case ChaseController.State.Discovery:
                UpdateDiscovery();
                break;
            case ChaseController.State.Chase:
            case ChaseController.State.Rush:
                UpdatePulse(chasePulseScale, chasePulseDuration);
                break;
        }
    }

    private void UpdateDiscovery()
    {
        // タイマー更新
        animTimer_ += Time.deltaTime;
        float halfTime = discoveryDuration * 0.5f;

        float scale;
        if (animTimer_ < halfTime)
        {
            // 前半: EaseOutBack でスケールアップ
            float t = animTimer_ / halfTime;
            scale = 1.0f + (discoveryScale - 1.0f) * Ease.Out.Back(t);
        }
        else if (animTimer_ < discoveryDuration)
        {
            // 後半: EaseOutBack で元スケールに戻す
            float t = (animTimer_ - halfTime) / halfTime;
            scale = discoveryScale + (1.0f - discoveryScale) * Ease.Out.Back(t);
        }
        else
        {
            // 演出終了: スケールをリセット
            transform.scale = initialScale_;
            return;
        }

        // スケール適用
        transform.scale = initialScale_ * scale;
    }

    private void UpdatePulse(float maxScale, float duration)
    {
        // タイマー更新・ループ正規化
        pulseTimer_ += Time.deltaTime;
        float t = (pulseTimer_ % duration) / duration;

        float scale;
        if (t < 0.5f)
        {
            // 前半: EaseOutBack でスケールアップ
            scale = 1.0f + (maxScale - 1.0f) * Ease.Out.Back(t / 0.5f);
        }
        else
        {
            // 後半: EaseOutBack で元スケールに戻す
            scale = maxScale + (1.0f - maxScale) * Ease.Out.Back((t - 0.5f) / 0.5f);
        }

        // スケール適用
        transform.scale = initialScale_ * scale;
    }
}
