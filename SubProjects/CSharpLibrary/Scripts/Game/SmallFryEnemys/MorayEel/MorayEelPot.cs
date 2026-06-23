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
    [SerializeField] private float fireScale = 2.0f;
    [SerializeField] private float fireScaleTimer = 0.5f;

    /* ----- 実行時状態 ----- */
    private float fireTimer_ = 0.0f; // 発射間隔の計測
    private float angleZ_ = 0.0f;    // 現在のZ回転角
    private Vector3 baseScale_;
    private float currentFireScaleTimer_ = -1.0f;

    public override void Initialize()
    {
        fireTimer_ = 0.0f;
        angleZ_ = 0.0f;
        baseScale_ = transform.scale;
        currentFireScaleTimer_ = -1.0f;
    }

    public override void Update()
    {
        // ツボを Z 軸回転させる
        angleZ_ += rotateZSpeed * Time.deltaTime;
        if (transform)
        {
            transform.rotate = Quaternion.FromEuler(new Vector3(0.0f, 0.0f, angleZ_));
        }

        // 発射時スケーリング演出
        UpdateFireScaleAnimation();

        // 一定間隔ごとに発射
        if (fireInterval <= 0.0f)
        {
            return;
        }

        fireTimer_ += Time.deltaTime;
        if (fireTimer_ >= fireInterval)
        {
            fireTimer_ -= fireInterval;
            FireMorayEel();
        }
    }

    private void UpdateFireScaleAnimation()
    {
        if (currentFireScaleTimer_ < 0.0f) { return; }

        currentFireScaleTimer_ += Time.deltaTime;
        float halfTime = fireScaleTimer * 0.5f;

        if (currentFireScaleTimer_ < halfTime)
        {
            // 前半: EaseOutBack でスケールアップ
            float t = currentFireScaleTimer_ / halfTime;
            float scale = 1.0f + (fireScale - 1.0f) * Ease.Out.Back(t);
            transform.scale = baseScale_ * scale;
        }
        else if (currentFireScaleTimer_ < fireScaleTimer)
        {
            // 後半: EaseOutBack で元のスケールに戻す
            float t = (currentFireScaleTimer_ - halfTime) / halfTime;
            float scale = fireScale + (1.0f - fireScale) * Ease.Out.Back(t);
            transform.scale = baseScale_ * scale;
        }
        else
        {
            // 演出終了
            transform.scale = baseScale_;
            currentFireScaleTimer_ = -1.0f;
        }
    }

    private void FireMorayEel()
    {
        Entity eel = ecsGroup.CreateEntity(morayEelPrefabName);
        if (!eel)
        {
            return;
        }

        // ツボと同じ位置・向きから発射する
        eel.transform.position = transform.position;
        eel.transform.rotate = transform.rotate;

        // 発射方向
        Vector3 fireDir = Matrix4x4.Transform(Vector3.up, Matrix4x4.Rotate(transform.rotate));
        Vector3 initialVelocity = fireDir * firePower;

        // 生成したウツボに初速を与え、発射
        MorayEelSpawnMove movement = eel.GetScript<MorayEelSpawnMove>();
        if (movement)
        {
            movement.Launch(initialVelocity);
        }

        // 発射時スケーリング演出を開始
        currentFireScaleTimer_ = 0.0f;
    }
}
