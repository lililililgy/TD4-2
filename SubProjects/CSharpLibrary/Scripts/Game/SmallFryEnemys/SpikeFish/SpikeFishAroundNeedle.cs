using System;

public class SpikeFishAroundNeedle : MonoScript
{
    /* ----- パラメータ ----- */
    [SerializeField] private float coolTime        = 3.0f;   // 発射後に0→1へイージングする時間
    [SerializeField] private float popInDuration   = 0.3f;   // coolTime終了直前、ポップインにかける秒数
    [SerializeField] private float maxAngularSpeed = 720.0f; // 発射直前の最大回転速度

    // この倍率を baseScale に掛けて適用する
    public float ScaleMult { get; private set; } = 0.0f;

    // 回復完了 = 攻撃可能
    public bool IsAppear => !isRecovering_ && ScaleMult >= 1.0f;

    /* ----- 実行時状態 ----- */
    private const string needleName_ = "Needle";
    private Vector3 baseScale_;
    private float   currentAngle_ = 0.0f;  // rad, チャージ中に加算される
    private float   angularVel_   = 0.0f;  // rad/s
    private float   chargeAccel_  = 0.0f;  // rad/s²
    private bool    isCharging_   = false;
    private bool    isRecovering_ = false;
    private float   recoverTimer_ = 0.0f;
    private SpriteAnimation spriteAnim_;

    public override void Initialize()
    {
        baseScale_ = transform.scale;
        spriteAnim_ = entity.GetScript<SpriteAnimation>();
        // スポーン時もスケール0→1でイージング
        BeginRecovery();
    }

    // SpikeFishAttack から発射直前に呼ぶ
    public void PlayFireAnimation()
    {
        spriteAnim_?.ResetAnimation();
        spriteAnim_?.Play();
    }

    // SpikeFishAttack からアニメーション終了時に元のUV位置へ戻す
    public void ResetFireAnimation()
    {
        spriteAnim_?.ResetAnimation();
    }

    public override void Update()
    {
        if (isCharging_) { UpdateCharging(); }
        if (isRecovering_) { UpdateRecovery(); }
    }

    // SpikeFishAttack からチャージ開始時に呼ぶ
    public void StartCharging(float chargeDuration)
    {
        isCharging_ = true;
        angularVel_ = 0.0f;
        float maxAngRad = maxAngularSpeed * Mathf.Deg2Rad;
        chargeAccel_ = maxAngRad / Math.Max(0.001f, chargeDuration);
    }

    // SpikeFishAttack からチャージ完了時に呼ぶ
    public void FireNeedle(int fireNeedleNum)
    {
        isCharging_ = false;
        angularVel_ = 0.0f;

        // 針の数の分だけ等間隔で角度を計算して生成する
        int count = Math.Max(1, fireNeedleNum);
        float intervalRad = (Mathf.PI * 2.0f) / count;
        for (int i = 0; i < count; i++)
        {
            // NeedleEntityを生成する
            Entity needle = ecsGroup.CreateEntity(needleName_);
            if (needle == null) { 
                continue;
            }

            // 針の方向を計算する
            float angle = currentAngle_ + i * intervalRad;
            Vector3 dir = new Vector3(Mathf.Cos(angle), Mathf.Sin(angle), 0.0f);

            // 針の位置と方向を設定する
            needle.transform.position = WorldPosition(transform);
            needle.GetScript<SpikeFishNeedle>()?.SetDirection(dir);
        }

        // 発射後はスケール0にして針の復活処理を開始
        ScaleMult = 0.0f;
        ApplyScale();
        BeginRecovery();
    }

    /* ----- private ----- */

    private static Vector3 WorldPosition(Transform t)
    {
        return new Vector3(t.matrix.m30, t.matrix.m31, t.matrix.m32);
    }

    private void BeginRecovery()
    {
        isRecovering_ = true;
        recoverTimer_ = 0.0f;
    }

    private void UpdateCharging()
    {
        // チャージ中は加速度で角速度を増加させ、角度を更新する
        angularVel_ += chargeAccel_ * Time.deltaTime;
        currentAngle_ += angularVel_  * Time.deltaTime;
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, currentAngle_);
    }

    private void UpdateRecovery()
    {
        recoverTimer_ += Time.deltaTime;

        if (recoverTimer_ >= coolTime)
        {
            ScaleMult     = 1.0f;
            isRecovering_ = false;
        }
        else
        {

            float popInStart = Math.Max(0.0f, coolTime - popInDuration);
            if (recoverTimer_ <= popInStart)
            {
                ScaleMult = 0.0f;
            }
            else
            {
                float t = (recoverTimer_ - popInStart) / Math.Max(0.001f, coolTime - popInStart);
                ScaleMult = Ease.Out.Back(t);
            }
        }

        ApplyScale();
    }

    private void ApplyScale()
    {
        transform.scale = baseScale_ * ScaleMult;
    }
}
