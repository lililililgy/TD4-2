using System;
using System.Collections.Generic;

//==========================================================
// キングクラゲの攻撃タイプ
//==========================================================
public enum KingJellyfishAttackTypeEnum
{
    ChargeAttack, //体当たり
    Omnidirectional_Beam, //全方向ビーム攻撃
    ElectricField, //帯電フィールド攻撃
    RotatingBeam, //回転ビーム攻撃
}

//================================================================
// キングクラゲのメインクラス
//================================================================
public class KingJellyfish : MonoScript {

    private const int DefaultlaserCount = 5;

    [SerializeField] public int maxHp = 10;
    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "Camera";

    //移動状態のパラメータ
    [SerializeField] public int actionLoopCount = 3;
    [SerializeField] public float moveDuration = 1.2f;
    [SerializeField] public float moveDistance = 300.0f;
    [SerializeField] public float moveSpeed = 250.0f;
    [SerializeField] public float moveArcHeight = 120.0f;
    [SerializeField] public float moveInertiaDuration = 0.5f;
    [SerializeField] public float moveInertiaRate = 0.35f;
    [SerializeField] public bool moveTowardTarget = true;

    //体当たり攻撃のパラメータ
    [SerializeField] public float idleDuration = 2.0f;
    [SerializeField] public float attackDuration = 1.0f;
    [SerializeField] public float chargeTellDuration = 0.8f;
    [SerializeField] public float chargeMoveDuration = 0.6f;
    [SerializeField] public float chargeSpeed = 500.0f;
    [SerializeField] public float chargeRecoveryDuration = 0.8f;
    [SerializeField] public float chargeInertiaRate = 0.5f;
    [SerializeField] public float chargePassThroughDistance = 300.0f;
    [SerializeField] public float chargeDamage = 20.0f;
    [SerializeField] public string chargeDamageFieldPrefabName = "JellyfishChargeDamageField";
    [SerializeField] public float chargeDamageFieldWidth = 180.0f;
    [SerializeField] public float chargeDamageFieldDuration = 0.6f;
    [SerializeField] public bool randomizeAttackType = true;
    [SerializeField] public KingJellyfishAttackTypeEnum fixedAttackType = KingJellyfishAttackTypeEnum.ChargeAttack;
    [SerializeField] public float chargeAttackWeight = 1.0f;
    [SerializeField] public float omnidirectionalBeamWeight = 1.0f;
    [SerializeField] public float electricFieldWeight = 1.0f;
    [SerializeField] public float rotatingBeamWeight = 1.0f;

    //8方向レーザー攻撃のパラメータ
    [SerializeField] public string laserPrefabName = "JellyfishLaser";
    [SerializeField] public float laserTellDuration = 0.8f;
    [SerializeField] public float laserFireDuration = 0.4f; // レーザーの発射時間
    [SerializeField] public float laserRecoveryDuration = 0.6f;
    [SerializeField] public float laserLength = 1200.0f;
    [SerializeField] public float laserWidth = 80.0f;
    [SerializeField] public float laserDamage = 15.0f;
    [SerializeField] public int laserCount = 8;

    //帯電フィールド攻撃のパラメータ
    [SerializeField] public string electricFieldPrefabName = "JellyfishElectricField";
    [SerializeField] public int electricFieldCount = 4;
    [SerializeField] public float electricFieldTellDuration = 1.0f;
    [SerializeField] public float electricFieldSpawnInterval = 0.2f;
    [SerializeField] public float electricFieldActiveDuration = 1.2f;
    [SerializeField] public float electricFieldRecoveryDuration = 0.5f;
    [SerializeField] public float electricFieldRadius = 140.0f;
    [SerializeField] public float electricFieldSpreadRadius = 300.0f;
    [SerializeField] public float electricFieldDamage = 12.0f;

    //回転ビーム攻撃のパラメータ
    [SerializeField] public int rotatingLaserCount = 4;
    [SerializeField] public float rotatingLaserTellDuration = 1.0f;
    [SerializeField] public float rotatingLaserDuration = 4.0f;
    [SerializeField] public float rotatingLaserRecoveryDuration = 0.5f;
    [SerializeField] public float rotatingLaserSpeed = 0.7f;
    [SerializeField] public float rotatingLaserLength = 1200.0f;
    [SerializeField] public float rotatingLaserWidth = 60.0f;
    [SerializeField] public float rotatingLaserDamage = 10.0f;

    private HP hp_;
    private IKingJellyfishState state_;
    private Entity targetEntity_;
    private Entity cameraEntity_;
    private bool attackRequested_;
    private int currentActionLoopCount_;

    private Vector2 chargeStartPosition_;
    private Vector2 chargeTargetPosition_;
    private Vector2 chargeMoveDirection_;
    private Vector2 chargeRecoveryVelocity_;
    private bool chargeDamageFieldDeployed_;
    private Vector2 arcMoveStartPosition_;
    private Vector2 arcMoveEndPosition_;
    private Vector2 arcMoveLastPosition_;
    private Vector2 arcMoveLastDirection_;
    private Vector2 arcMoveInertiaVelocity_;
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
        ResetActionLoop();
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
        if (!randomizeAttackType)
        {
            return fixedAttackType;
        }

        float chargeWeight = chargeAttackWeight > 0.0f ? chargeAttackWeight : 0.0f;
        float beamWeight = omnidirectionalBeamWeight > 0.0f ? omnidirectionalBeamWeight : 0.0f;
        float fieldWeight = electricFieldWeight > 0.0f ? electricFieldWeight : 0.0f;
        float rotatingWeight = rotatingBeamWeight > 0.0f ? rotatingBeamWeight : 0.0f;
        float totalWeight = chargeWeight + beamWeight + fieldWeight + rotatingWeight;
        if (totalWeight <= 0.0f)
        {
            return fixedAttackType;
        }

        float lottery = RandomUtil.NextFloat() * totalWeight;
        if (lottery < chargeWeight)
        {
            return KingJellyfishAttackTypeEnum.ChargeAttack;
        }

        lottery -= chargeWeight;
        if (lottery < beamWeight)
        {
            return KingJellyfishAttackTypeEnum.Omnidirectional_Beam;
        }

        lottery -= beamWeight;
        if (lottery < fieldWeight)
        {
            return KingJellyfishAttackTypeEnum.ElectricField;
        }

        return KingJellyfishAttackTypeEnum.RotatingBeam;
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
    // 行動ループ処理
    //=============================================================
    internal void ResetActionLoop()
    {
        currentActionLoopCount_ = 0;
    }

    internal bool ConsumeActionLoop()
    {
        currentActionLoopCount_++;
        return currentActionLoopCount_ < ActionLoopCount;
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

    //=============================
    // 体当たり攻撃の開始処理
    //=============================
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
        chargeMoveDirection_ = direction;
        chargeRecoveryVelocity_ = direction * (ChargeSpeed * ChargeInertiaRate);
        chargeDamageFieldDeployed_ = false;
        movementDepth_ = transform.position.z;
        RotateTowardPosition(chargeTargetPosition_);
        return true;
    }

    //=============================
    // 体当たり攻撃の予備動作処理
    //=============================
    internal void UpdateChargeTell()
    {
        // 設定されていない場合、ターゲットの設定
        ResolveTarget();

        // ターゲットが存在する場合のみ回転処理を行う
        if (targetEntity_ != null && targetEntity_.transform != null)
        {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));


            
            //デバッグ::カラーを点滅させる
            SpriteRenderer spriteRenderer = entity.GetComponent<SpriteRenderer>();
            if (spriteRenderer != null)
            {
                float blinkFrequency = 5.0f; // 点滅の周波数（Hz）
                float blinkPhase = Mathf.Sin(Time.time * blinkFrequency * Mathf.PI * 2.0f);
                float alpha = Mathf.Clamp01((blinkPhase + 1.0f) * 0.5f); // 0から1の範囲に変換
                Vector4 originalColor = spriteRenderer.color;
                spriteRenderer.color = new Vector4(1.0f, 0.0f, 0.0f, alpha);
            }
        }
    }

    internal bool UpdateChargeAttack()
    {

        SpriteRenderer spriteRenderer = entity.GetComponent<SpriteRenderer>();
        spriteRenderer.color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);

        // ターゲットの位置に向かって移動する
        Vector2 currentPosition = ToPlane(transform.position);
        Vector2 toTarget = chargeTargetPosition_ - currentPosition;
        float remainingDistance = toTarget.Length();

        // 目標位置に到達したかどうかを判定する
        if (remainingDistance <= 0.001f)
        {
            SetPlanePosition(chargeTargetPosition_);
            DeployChargeDamageField();
            return true;
        }

        // 移動距離を計算し、目標位置に到達するかどうかを判定する
        float moveDistance = ChargeSpeed * Time.deltaTime;
        if (moveDistance >= remainingDistance)
        {
            SetPlanePosition(chargeTargetPosition_);
            RotateTowardPosition(chargeTargetPosition_);
            DeployChargeDamageField();
            return true;
        }

        // 移動距離が残り距離よりも小さい場合は、移動する
        Vector2 next = currentPosition + chargeMoveDirection_ * moveDistance;
        SetPlanePosition(next);

        // 移動中もターゲットの位置に向かって回転する
        RotateTowardPosition(chargeTargetPosition_);
        return false;
    }

    //=============================
    // 体当たり攻撃の回復処理
    //=============================
    internal void UpdateChargeRecovery(float elapsed)
    {
        float duration = ChargeRecoveryDuration;
        float dampingRatio = 1.0f - Mathf.Clamp01(elapsed / duration);
        Vector2 currentPosition = ToPlane(transform.position);
        Vector2 next = currentPosition + chargeRecoveryVelocity_ * dampingRatio * Time.deltaTime;
        SetPlanePosition(next);

        // 回復中もターゲットの位置に向かって回転する
        RotateTowardPosition(chargeTargetPosition_);
    }

    //=============================
    // 体当たり攻撃のダメージフィールド生成処理
    //=============================
    internal void DeployChargeDamageField()
    {
        if (chargeDamageFieldDeployed_)
        {
            return;
        }

        chargeDamageFieldDeployed_ = true;
        CreateChargeDamageField(chargeStartPosition_, chargeTargetPosition_);
    }

    private void CreateChargeDamageField(Vector2 start, Vector2 end)
    {
        if (String.IsNullOrEmpty(chargeDamageFieldPrefabName))
        {
            return;
        }

        Entity damageField = ecsGroup.CreateEntity(chargeDamageFieldPrefabName);
        if (damageField == null)
        {
            return;
        }

        ConfigureChargeDamageField(damageField, start, end);
    }

    private void ConfigureChargeDamageField(Entity damageField, Vector2 start, Vector2 end)
    {
        Vector2 move = end - start;
        float length = move.Length();
        if (length <= 0.001f)
        {
            damageField.Destroy();
            return;
        }

        Vector2 direction = move.Normalized();
        Vector2 center = start + direction * (length * 0.5f);
        damageField.transform.position = new Vector3(center.x, center.y, transform.position.z);
        damageField.transform.scale = new Vector3(ChargeDamageFieldWidth, length, 1.0f);
        float angle = Mathf.Atan2(direction.x, direction.y);
        damageField.transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);

        JellyfishChargeDamageField damageFieldScript = damageField.GetScript<JellyfishChargeDamageField>();
        if (damageFieldScript != null)
        {
            damageFieldScript.Configure(start, end, ChargeDamageFieldWidth, chargeDamage, ChargeDamageFieldDuration, transform.position.z);
        }

        AttackCollision attackCollision = damageField.GetScript<AttackCollision>();
        if (attackCollision != null)
        {
            attackCollision.Damage = chargeDamage;
        }
    }

    //=============================
    // 弧を描く移動処理
    //=============================
    internal void BeginArcMove()
    {
        movementDepth_ = transform.position.z;
        arcMoveStartPosition_ = ToPlane(transform.position);

        Vector2 moveDirection = SelectArcMoveDirection();
        arcMoveEndPosition_ = arcMoveStartPosition_ + moveDirection * MoveDistance;
        arcMoveLastPosition_ = arcMoveStartPosition_;
        arcMoveLastDirection_ = moveDirection;
        arcMoveInertiaVelocity_ = moveDirection * (MoveSpeed * MoveInertiaRate);
        RotateTowardPosition(arcMoveEndPosition_);
    }

    internal bool UpdateArcMove(float elapsed)
    {
        float duration = MoveDuration;
        float ratio = Mathf.Clamp01(elapsed / duration);

        Vector2 linePosition = Vector2.Lerp(arcMoveStartPosition_, arcMoveEndPosition_, ratio);
        float arcOffset = Mathf.Sin(ratio * Mathf.PI) * MoveArcHeight;
        Vector2 next = new Vector2(linePosition.x, linePosition.y + arcOffset);

        Vector2 frameMove = next - arcMoveLastPosition_;
        if (frameMove.LengthSq() > 0.001f)
        {
            arcMoveLastDirection_ = frameMove.Normalized();
            arcMoveInertiaVelocity_ = arcMoveLastDirection_ * (MoveSpeed * MoveInertiaRate);
        }
        arcMoveLastPosition_ = next;

        SetPlanePosition(next);

        RotateTowardPosition(arcMoveEndPosition_);
        return ratio >= 1.0f;
    }

    internal void UpdateArcMoveInertia(float elapsed)
    {
        float duration = MoveInertiaDuration;
        float dampingRatio = 1.0f - Mathf.Clamp01(elapsed / duration);
        Vector2 currentPosition = ToPlane(transform.position);
        Vector2 next = currentPosition + arcMoveInertiaVelocity_ * dampingRatio * Time.deltaTime;
        SetPlanePosition(next);

        RotateTowardPosition(next + arcMoveLastDirection_);
    }

    //=============================
    // 8方向レーザー攻撃処理
    //=============================
    internal void UpdateLaserTell()
    {
        ResolveTarget();
        if (targetEntity_ != null && targetEntity_.transform != null)
        {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));

            //カラーを点滅させる
            SpriteRenderer spriteRenderer = entity.GetComponent<SpriteRenderer>();
            if (spriteRenderer != null)
            {
                float blinkFrequency = 5.0f; // 点滅の周波数（Hz）
                float blinkPhase = Mathf.Sin(Time.time * blinkFrequency * Mathf.PI * 2.0f);
                float alpha = Mathf.Clamp01((blinkPhase + 1.0f) * 0.5f); // 0から1の範囲に変換
                Vector4 originalColor = spriteRenderer.color;
                spriteRenderer.color = new Vector4(0.0f, 1.0f, 0.0f, alpha);
            }
        }
    }

    //=============================
    // 8方向レーザー攻撃の発射処理
    //=============================
    internal void FireOmnidirectionalLaser()
    {
        SpriteRenderer spriteRenderer = entity.GetComponent<SpriteRenderer>();
        spriteRenderer.color = new Vector4(0.0f, 1.0f, 0.0f, 1.0f);

        if (String.IsNullOrEmpty(laserPrefabName))
        {
            return;
        }

        Vector2 origin = ToPlane(transform.position);
        int count = LaserCount;

        // レーザーを全方向に発射する
        for (int i = 0; i < count; i++)
        {
            float angle = Mathf.PI * 2.0f * i / count;
            Vector2 direction = new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));

            // レーザーエンティティを生成する
            Entity laser = ecsGroup.CreateEntity(laserPrefabName);
            if (laser == null)
            {
                continue;
            }

            ConfigureLaserEntity(laser, origin, direction, LaserLength, LaserWidth, laserDamage, LaserFireDuration, 0.0f);
        }
    }

    //=============================
    // 回転ビーム攻撃
    //=============================
    internal void FireRotatingLasers()
    {
        if (String.IsNullOrEmpty(laserPrefabName))
        {
            return;
        }

        Vector2 origin = ToPlane(transform.position);
        int count = RotatingLaserCount;
        for (int i = 0; i < count; i++)
        {
            float angle = Mathf.PI * 2.0f * i / count;
            Vector2 direction = new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));
            Entity laser = ecsGroup.CreateEntity(laserPrefabName);
            if (laser == null)
            {
                continue;
            }

            ConfigureLaserEntity(
                laser,
                origin,
                direction,
                RotatingLaserLength,
                RotatingLaserWidth,
                rotatingLaserDamage,
                RotatingLaserDuration,
                rotatingLaserSpeed);
        }
    }

    //=============================
    // 帯電フィールド攻撃
    //=============================
    internal void DeployElectricFields()
    {
        if (String.IsNullOrEmpty(electricFieldPrefabName))
        {
            return;
        }

        ResolveTarget();
        Vector2 center = targetEntity_ != null && targetEntity_.transform != null
            ? ToPlane(targetEntity_.transform.position)
            : ToPlane(transform.position);

        for (int i = 0; i < ElectricFieldCount; i++)
        {
            Vector2 position = center;
            if (i > 0)
            {
                float angle = RandomUtil.NextFloat() * Mathf.PI * 2.0f;
                float distance = RandomUtil.NextFloat() * electricFieldSpreadRadius;
                position += new Vector2(Mathf.Cos(angle), Mathf.Sin(angle)) * distance;
            }

            Entity field = ecsGroup.CreateEntity(electricFieldPrefabName);
            if (field == null)
            {
                continue;
            }

            JellyfishElectricField fieldScript = field.GetScript<JellyfishElectricField>();
            if (fieldScript == null)
            {
                field.Destroy();
                continue;
            }

            float activationDelay = ElectricFieldTellDuration + ElectricFieldSpawnInterval * i;
            fieldScript.Configure(
                position,
                ElectricFieldRadius,
                electricFieldDamage,
                activationDelay,
                ElectricFieldActiveDuration,
                transform.position.z);
        }
    }

    internal void UpdateLaserRecovery()
    {
        ResolveTarget();
        if (targetEntity_ != null && targetEntity_.transform != null)
        {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));
        }
    }

    private void ConfigureLaserEntity(
        Entity laser,
        Vector2 origin,
        Vector2 direction,
        float length,
        float width,
        float damage,
        float duration,
        float rotationSpeed)
    {
        Vector2 normalized = direction.Normalized();
        if (normalized.LengthSq() <= 0.001f)
        {
            normalized = Vector2.up;
        }

        JellyfishLaser laserScript = laser.GetScript<JellyfishLaser>();
        if (laserScript == null)
        {
            laser.Destroy();
            return;
        }

        laserScript.Configure(origin, normalized, length, width, damage, duration, transform.position.z, rotationSpeed);
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

    internal float ChargeSpeed
    {
        get { return chargeSpeed > 0 ? chargeSpeed : 1.0f; }
    }

    internal float ChargeInertiaRate
    {
        get { return chargeInertiaRate > 0.0f ? chargeInertiaRate : 0.0f; }
    }

    internal float ChargeRecoveryDuration
    {
        get { return chargeRecoveryDuration > 0 ? chargeRecoveryDuration : 0.01f; }
    }

    internal float ChargeDamageFieldWidth
    {
        get { return chargeDamageFieldWidth > 0.0f ? chargeDamageFieldWidth : 1.0f; }
    }

    internal float ChargeDamageFieldDuration
    {
        get { return chargeDamageFieldDuration > 0.0f ? chargeDamageFieldDuration : ChargeMoveDuration; }
    }

    internal float IdleDuration
    {
        get { return idleDuration > 0 ? idleDuration : 0.01f; }
    }

    internal int ActionLoopCount
    {
        get { return actionLoopCount > 0 ? actionLoopCount : 1; }
    }

    internal float MoveDuration
    {
        get { return moveDuration > 0.0f ? moveDuration : 0.01f; }
    }

    internal float MoveDistance
    {
        get { return moveDistance > 0.0f ? moveDistance : 1.0f; }
    }

    internal float MoveSpeed
    {
        get { return moveSpeed > 0.0f ? moveSpeed : 1.0f; }
    }

    internal float MoveArcHeight
    {
        get { return moveArcHeight; }
    }

    internal float MoveInertiaDuration
    {
        get { return moveInertiaDuration > 0.0f ? moveInertiaDuration : 0.01f; }
    }

    internal float MoveInertiaRate
    {
        get { return moveInertiaRate > 0.0f ? moveInertiaRate : 0.0f; }
    }

    internal float LaserTellDuration
    {
        get { return laserTellDuration > 0.0f ? laserTellDuration : 0.01f; }
    }

    internal float LaserFireDuration
    {
        get { return laserFireDuration > 0.0f ? laserFireDuration : 0.01f; }
    }

    internal float LaserRecoveryDuration
    {
        get { return laserRecoveryDuration > 0.0f ? laserRecoveryDuration : 0.01f; }
    }

    internal float LaserLength
    {
        get { return laserLength > 0.0f ? laserLength : 1.0f; }
    }

    internal float LaserWidth
    {
        get { return laserWidth > 0.0f ? laserWidth : 1.0f; }
    }

    internal int LaserCount
    {
        get { return laserCount > 0 ? laserCount : 8; }
    }

    internal int ElectricFieldCount
    {
        get { return electricFieldCount > 0 ? electricFieldCount : 1; }
    }

    internal float ElectricFieldTellDuration
    {
        get { return electricFieldTellDuration > 0.0f ? electricFieldTellDuration : 0.01f; }
    }

    internal float ElectricFieldSpawnInterval
    {
        get { return electricFieldSpawnInterval > 0.0f ? electricFieldSpawnInterval : 0.0f; }
    }

    internal float ElectricFieldActiveDuration
    {
        get { return electricFieldActiveDuration > 0.0f ? electricFieldActiveDuration : 0.01f; }
    }

    internal float ElectricFieldRecoveryDuration
    {
        get { return electricFieldRecoveryDuration > 0.0f ? electricFieldRecoveryDuration : 0.01f; }
    }

    internal float ElectricFieldRadius
    {
        get { return electricFieldRadius > 0.0f ? electricFieldRadius : 1.0f; }
    }

    internal int RotatingLaserCount
    {
        get { return rotatingLaserCount > 0 ? rotatingLaserCount : 1; }
    }

    internal float RotatingLaserTellDuration
    {
        get { return rotatingLaserTellDuration > 0.0f ? rotatingLaserTellDuration : 0.01f; }
    }

    internal float RotatingLaserDuration
    {
        get { return rotatingLaserDuration > 0.0f ? rotatingLaserDuration : 0.01f; }
    }

    internal float RotatingLaserRecoveryDuration
    {
        get { return rotatingLaserRecoveryDuration > 0.0f ? rotatingLaserRecoveryDuration : 0.01f; }
    }

    internal float RotatingLaserLength
    {
        get { return rotatingLaserLength > 0.0f ? rotatingLaserLength : LaserLength; }
    }

    internal float RotatingLaserWidth
    {
        get { return rotatingLaserWidth > 0.0f ? rotatingLaserWidth : LaserWidth; }
    }

    private Vector2 SelectArcMoveDirection()
    {
        if (moveTowardTarget)
        {
            ResolveTarget();
            if (targetEntity_ != null && targetEntity_.transform != null)
            {
                Vector2 toTarget = ToPlane(targetEntity_.transform.position) - ToPlane(transform.position);
                if (toTarget.LengthSq() > 0.001f)
                {
                    return toTarget.Normalized();
                }
            }
        }

        float angle = RandomUtil.NextFloat() * Mathf.PI * 2.0f;
        return new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));
    }


    //=============================
    // ターゲットの解決処理
    //=============================
    private void ResolveTarget()
    {
        if (targetEntity_ == null && !String.IsNullOrEmpty(targetEntityName))
        {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
    }
}
