using System;
using System.Collections.Generic;

//==========================================================
// キングクラゲの攻撃タイプ
//==========================================================
public enum KingJellyfishAttackTypeEnum
{
    ChargeAttack, //体当たり
    Omnidirectional_Beam, //全方向ビーム攻撃
}

//================================================================
// キングクラゲのメインクラス
//================================================================
public class KingJellyfish : MonoScript {

    private const int DefaultlaserCount = 5;

    [SerializeField] public int maxHp = 10;
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "Camera";

    //体当たり攻撃のパラメータ
    [SerializeField] public float idleDuration = 2.0f;
    [SerializeField] public float attackDuration = 1.0f;
    [SerializeField] public float chargeTellDuration = 0.8f;
    [SerializeField] public float chargeMoveDuration = 0.6f;
    [SerializeField] public float chargeRecoveryDuration = 0.8f;
    [SerializeField] public float chargePassThroughDistance = 300.0f;
    [SerializeField] public float chargeDamage = 20.0f;
    [SerializeField] public KingJellyfishAttackTypeEnum fixedAttackType = KingJellyfishAttackTypeEnum.ChargeAttack;

    private HP hp_;
    private IKingJellyfishState state_;
    private Entity targetEntity_;
    private Entity cameraEntity_;
    private bool attackRequested_;

    private Vector2 chargeStartPosition_;
    private Vector2 chargeTargetPosition_;
    private float movementDepth_;

    //=============================
    // 初期化
    //=============================
    public override void Initialize() {

        // HPコンポーネントを取得または追加
        hp_ = entity.GetScript<HP>();
        if(hp_ == null)
        {
            hp_ = entity.AddScript<HP>();
        }
        hp_.MaxHp = maxHp > 0 ? maxHp : 1;
        hp_.Initialize();

        // 初期状態を待機状態に設定
        if (!String.IsNullOrEmpty(targetEntityName))
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
        if (!String.IsNullOrEmpty(cameraEntityName))
        {
            cameraEntity_ = ecsGroup.FindEntity(cameraEntityName);
        }

        movementDepth_ = transform.position.z;
        attackRequested_ = false;
        ChangeState(new KingJellyfishIdleState());
    }

    //=============================
    // 更新
    //=============================
    public override void Update() {

        if (state_ != null)
        {
            state_.Update(this);
        }

    }

    //=============================================================
    // ターゲットの設定
    //=============================================================
    public void SetTarget(Entity target)
    {
        targetEntity_ = target;
    }

    //=============================================================
    // 攻撃の要求
    //=============================================================
    public void RequestAttack()
    {
        attackRequested_ = true;
    }

    //=============================================================
    // ダメージ処理
    //=============================================================
    public void TakeDamage(float damage)
    {
        if (hp_ == null)
        {
            return;
        }

        hp_.TakeDamage(damage);
    }

    //=============================================================
    // 攻撃タイプの選択
    //=============================================================
    internal KingJellyfishAttackTypeEnum SelectAttackType()
    {   
        // Omnidirectional_Beam は未実装なので、現状は固定攻撃だけを返す。
        return fixedAttackType;
    }

    //=============================================================
    // 攻撃リクエスト処理
    //=============================================================
    internal bool ConsumeAttackRequest()
    {
        bool requested = attackRequested_;
        attackRequested_ = false;
        return requested;
    }

    //=============================================================
    // 状態の変更
    //=============================================================
    internal void ChangeState(IKingJellyfishState nextState)
    {
        if (state_ != null)
        {
            state_.Exit(this);
        }

        state_ = nextState;
        if (state_ != null)
        {
            state_.Enter(this);
        }
    }

    internal bool BeginChargeAttack()
    {
        ResolveTarget();
        if (targetEntity_ == null || targetEntity_.transform == null)
        {
            return false;
        }

        Vector2 start = ToPlane(transform.position);
        Vector2 target = ToPlane(targetEntity_.transform.position);
        Vector2 direction = (target - start).Normalized();
        if (direction.LengthSq() <= 0.001f)
        {
            direction = Vector2.down;
        }

        chargeStartPosition_ = start;
        chargeTargetPosition_ = target + direction * chargePassThroughDistance;
        movementDepth_ = transform.position.z;
        RotateTowardPosition(chargeTargetPosition_);
        return true;
    }

    internal void UpdateChargeTell()
    {
        ResolveTarget();
        if (targetEntity_ != null && targetEntity_.transform != null)
        {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));
        }
    }

    internal void UpdateChargeAttack(float elapsed)
    {
        float duration = chargeMoveDuration > 0.0f ? chargeMoveDuration : 0.001f;
        float ratio = Mathf.Clamp01(elapsed / duration);

        Vector2 next = Vector2.Lerp(chargeStartPosition_, chargeTargetPosition_, ratio);
        SetPlanePosition(next);

        RotateTowardPosition(chargeTargetPosition_);
    }

    internal void UpdateChargeRecovery()
    {
        RotateTowardPosition(chargeTargetPosition_);
    }


    private void SetPlanePosition(Vector2 position)
    {
        transform.position = new Vector3(position.x, position.y, movementDepth_);
    }
    private static Vector2 ToPlane(Vector3 position)
    {
        return new Vector2(position.x, position.y);
    }

    //============================
    // ターゲットの位置に向かって回転する処理
    //============================
    private void RotateTowardPosition(Vector2 targetPosition)
    {
        Vector2 direction = targetPosition - ToPlane(transform.position);
        if (direction.Length() <= 0.001f)
        {
            return;
        }

        Vector2 normalized = direction.Normalized();
        float angle = Mathf.Atan2(normalized.x, normalized.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
    }

    internal float ChargeTellDuration
    {
        get { return chargeTellDuration > 0 ? chargeTellDuration : 0.01f; }
    }   

    internal float ChargeMoveDuration
    {
        get { return chargeMoveDuration > 0 ? chargeMoveDuration : 0.01f; }
    }

    internal float ChargeRecoveryDuration
    {
        get { return chargeRecoveryDuration > 0 ? chargeRecoveryDuration : 0.01f; }
    }

    internal float IdleDuration
    {
        get { return idleDuration > 0 ? idleDuration : 0.01f; }
    }

    private void ResolveTarget()
    {
        if (targetEntity_ == null && !String.IsNullOrEmpty(targetEntityName))
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
    }
}
