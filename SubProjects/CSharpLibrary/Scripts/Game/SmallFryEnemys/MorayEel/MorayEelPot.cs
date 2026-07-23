using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class MorayEelPot : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private float firePower = 0.0f;
    [SerializeField] private float rotateZSpeed = 0.0f;
    [SerializeField] private float fireInterval = 0.0f;
    [SerializeField] private string morayEelPrefabName = "MorayEel";

    // スケーリング演出
    [SerializeField] private Vector3 fireShrinkScale    = new Vector3(0.7f, 0.7f, 0.7f);
    [SerializeField] private Vector3 fireExpandScale    = new Vector3(2.0f, 2.0f, 2.0f);
    [SerializeField] private float   fireShrinkDuration = 0.1f;
    [SerializeField] private float   fireExpandDuration = 0.2f;
    [SerializeField] private float   fireReturnDuration = 0.2f;

    /* ----- 実行時状態 ----- */
    private float   fireTimer_ = 0.0f; // 発射間隔の計測
    private float   angleZ_    = 0.0f; // 現在のZ回転角
    private Vector3 baseScale_;

    private enum FireScalePhase { IDLE, SHRINK, EXPAND, RETURN }
    private FireScalePhase fireScalePhase_   = FireScalePhase.IDLE;
    private float          fireScaleElapsed_ = 0.0f;

    private Action[] fireScaleActions_;

    public override void Initialize()
    {
        if (transform == null) { return; }

        fireTimer_        = 0.0f;
        angleZ_           = 0.0f;
        baseScale_        = transform != null ? transform.scale : Vector3.one;
        fireScalePhase_   = FireScalePhase.IDLE;
        fireScaleElapsed_ = 0.0f;

        fireScaleActions_ = new Action[]
        {
            null,          // IDLE
            UpdateShrink,  // SHRINK
            UpdateExpand,  // EXPAND
            UpdateReturn,  // RETURN
        };
    }

    public override void Update()
    {
        if (transform == null) { return; }

        // ツボを Z 軸回転させる
        angleZ_ += rotateZSpeed * Time.deltaTime;
        transform.rotate = Quaternion.FromEuler(new Vector3(0.0f, 0.0f, angleZ_));

        // 発射時スケーリング演出
        UpdateFireScaleAnimation();

        // 一定間隔ごとに発射
        if (fireInterval <= 0.0f) { 
            return;
        }

        fireTimer_ += Time.deltaTime;
        if (fireTimer_ >= fireInterval)
        {
            // 発射時スケーリング演出を開始
            fireTimer_ = 0.0f;
            fireScalePhase_ = FireScalePhase.SHRINK;
            fireScaleElapsed_ = 0.0f;
        }
    }

    private void UpdateFireScaleAnimation()
    {
        if (fireScalePhase_ == FireScalePhase.IDLE) {
            return; 
        }
        fireScaleElapsed_ += Time.deltaTime;
        fireScaleActions_[(int)fireScalePhase_]?.Invoke();
    }

    private void UpdateShrink()
    {
        // 縮む
        float t = Mathf.Clamp01(fireScaleElapsed_ / fireShrinkDuration);
        float eased = Ease.Out.Back(t);
        Vector3 shrinkScale = new Vector3(
            baseScale_.x * fireShrinkScale.x,
            baseScale_.y * fireShrinkScale.y,
            baseScale_.z * fireShrinkScale.z
        );
        transform.scale = Vector3.Lerp(baseScale_, shrinkScale, eased);
        if (fireScaleElapsed_ >= fireShrinkDuration) { NextPhase(); }
    }

    private void UpdateExpand()
    {
        // EaseOutBack でスケールアップ
        float t = Mathf.Clamp01(fireScaleElapsed_ / fireExpandDuration);
        Vector3 shrunkScale = new Vector3(
            baseScale_.x * fireShrinkScale.x,
            baseScale_.y * fireShrinkScale.y,
            baseScale_.z * fireShrinkScale.z
        );
        Vector3 expandScale = new Vector3(
            baseScale_.x * fireExpandScale.x,
            baseScale_.y * fireExpandScale.y,
            baseScale_.z * fireExpandScale.z
        );
        transform.scale = Vector3.Lerp(shrunkScale, expandScale, Ease.Out.Back(t));
        if (fireScaleElapsed_ >= fireExpandDuration) { NextPhase(); }
    }

    private void UpdateReturn()
    {
        // EaseOutBack で元のスケールに戻す
        float t = Mathf.Clamp01(fireScaleElapsed_ / fireReturnDuration);
        Vector3 expandScale = new Vector3(
            baseScale_.x * fireExpandScale.x,
            baseScale_.y * fireExpandScale.y,
            baseScale_.z * fireExpandScale.z
        );
        transform.scale = Vector3.Lerp(expandScale, baseScale_, Ease.Out.Back(t));
        if (fireScaleElapsed_ >= fireReturnDuration)
        {
            // 演出終了
            transform.scale   = baseScale_;
            fireScalePhase_   = FireScalePhase.IDLE;
            fireScaleElapsed_ = 0.0f;
        }
    }

    private void NextPhase()
    {
        if (fireScalePhase_== FireScalePhase.SHRINK)
        {
            FireMorayEel();
        }
        fireScalePhase_   = (FireScalePhase)((int)fireScalePhase_ + 1);
        fireScaleElapsed_ = 0.0f;
    }

    private void FireMorayEel()
    {
        if (transform == null || ecsGroup == null) { return; }

        Entity eel = ecsGroup.CreateEntity(morayEelPrefabName);
        if (!eel || eel.transform == null) { return; }

        // ツボと同じ位置・向きから発射する
        eel.transform.position = transform.position;
        eel.transform.rotate   = transform.rotate;

        // 発射方向
        Vector3 fireDir         = Matrix4x4.Transform(Vector3.up, Matrix4x4.Rotate(transform.rotate));
        Vector3 initialVelocity = fireDir * firePower;

        // 生成したウツボに初速を与え、発射
        MorayEelSpawnMove movement = eel.GetScript<MorayEelSpawnMove>();
        if (movement) { movement.Launch(initialVelocity); }

        // 発射時スケーリング演出を開始
        fireScalePhase_   = FireScalePhase.SHRINK;
        fireScaleElapsed_ = 0.0f;
    }
}
