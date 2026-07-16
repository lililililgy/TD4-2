using System;
using System.Collections.Generic;

//==========================================================
// キングクラゲの攻撃タイプ
//==========================================================
public enum KingJellyfishAttackTypeEnum {
    ChargeAttack, //体当たり
    Omnidirectional_Beam, //全方向ビーム攻撃
    ElectricField, //帯電フィールド攻撃
    RotatingBeam, //回転ビーム攻撃
}

//================================================================
// キングクラゲのメインクラス
//================================================================
public class KingJellyfish : MonoScript {

    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public string cameraEntityName = "Camera";

    [SerializeField] public float idleDuration = 2.0f;
    [SerializeField] public float damageInvincibilityDuration = 0.5f;
    [SerializeField] public bool randomizeAttackType = true;
    [SerializeField] public KingJellyfishAttackTypeEnum fixedAttackType = KingJellyfishAttackTypeEnum.ChargeAttack;

    private HP hp_;
    private IKingJellyfishState state_;
    private Entity targetEntity_;
    private Entity cameraEntity_;
    private bool attackRequested_;
    private int currentActionLoopCount_;
    private KingJellyfishMoveSettings moveSettings_;
    private KingJellyfishChargeAttackSettings chargeSettings_;
    private KingJellyfishOmnidirectionalBeamSettings beamSettings_;
    private KingJellyfishElectricFieldSettings electricFieldSettings_;
    private KingJellyfishRotatingBeamSettings rotatingBeamSettings_;
    private JellyfishWeakPoint weakPoint_;
    private float damageInvincibilityRemaining_;

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
        hp_ = entity.GetScript<HP>();

        moveSettings_ = GetOrAddSettings<KingJellyfishMoveSettings>();
        chargeSettings_ = GetOrAddSettings<KingJellyfishChargeAttackSettings>();
        beamSettings_ = GetOrAddSettings<KingJellyfishOmnidirectionalBeamSettings>();
        electricFieldSettings_ = GetOrAddSettings<KingJellyfishElectricFieldSettings>();
        rotatingBeamSettings_ = GetOrAddSettings<KingJellyfishRotatingBeamSettings>();
        ResolveWeakPoint();
        SetWeakPointCollisionEnabled(true);

        // 初期状態を待機状態に設定
        if (!String.IsNullOrEmpty(targetEntityName)) {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
        if (!String.IsNullOrEmpty(cameraEntityName)) {
            cameraEntity_ = ecsGroup.FindEntity(cameraEntityName);
        }

        movementDepth_ = transform.position.z;
        attackRequested_ = false;
        damageInvincibilityRemaining_ = 0.0f;
        ResetActionLoop();
        ChangeState(new KingJellyfishIdleState());
    }

    //=============================
    // 更新
    //=============================
    public override void Update() {
        UpdateDamageInvincibility();

        if (state_ != null) {
            state_.Update(this);
        }

    }

    //=============================================================
    // ターゲットの設定
    //=============================================================
    public void SetTarget(Entity target) {
        targetEntity_ = target;
    }

    //=============================================================
    // 攻撃の要求
    //=============================================================
    public void RequestAttack() {
        attackRequested_ = true;
    }

    //=============================================================
    // ダメージ処理
    //=============================================================
    public void TakeDamage(float damage) {
        if (hp_ == null || damage <= 0.0f || damageInvincibilityRemaining_ > 0.0f) {
            return;
        }

        float hpBeforeDamage = hp_.CurrentHp;
        hp_.TakeDamage(damage);
        if (hp_.CurrentHp < hpBeforeDamage) {
            damageInvincibilityRemaining_ = NonNegative(damageInvincibilityDuration);
        }
    }

    private void UpdateDamageInvincibility() {
        if (damageInvincibilityRemaining_ <= 0.0f) {
            return;
        }

        damageInvincibilityRemaining_ -= Time.deltaTime;
        if (damageInvincibilityRemaining_ < 0.0f) {
            damageInvincibilityRemaining_ = 0.0f;
        }
    }

    internal void SetWeakPointCollisionEnabled(bool enabled) {
        if (weakPoint_ == null) {
            ResolveWeakPoint();
        }

        if (weakPoint_ != null) {
            weakPoint_.SetCollisionEnabled(enabled);
        }
    }

    private void ResolveWeakPoint() {
        weakPoint_ = null;
        uint childCount = entity.GetChildCount();
        for (uint i = 0; i < childCount; i++) {
            Entity child = entity.GetChild(i);
            if (child == null) {
                continue;
            }

            JellyfishWeakPoint weakPoint = child.GetScript<JellyfishWeakPoint>();
            if (weakPoint != null) {
                weakPoint_ = weakPoint;
                return;
            }
        }
    }

    //=============================================================
    // 攻撃タイプの選択
    //=============================================================
    internal KingJellyfishAttackTypeEnum SelectAttackType() {
        if (!randomizeAttackType) {
            return fixedAttackType;
        }

        float chargeWeight = NonNegative(chargeSettings_.selectionWeight);
        float beamWeight = NonNegative(beamSettings_.selectionWeight);
        float fieldWeight = NonNegative(electricFieldSettings_.selectionWeight);
        float rotatingWeight = NonNegative(rotatingBeamSettings_.selectionWeight);
        float totalWeight = chargeWeight + beamWeight + fieldWeight + rotatingWeight;
        if (totalWeight <= 0.0f) {
            return fixedAttackType;
        }

        float lottery = RandomUtil.NextFloat() * totalWeight;
        if (lottery < chargeWeight) {
            return KingJellyfishAttackTypeEnum.ChargeAttack;
        }

        lottery -= chargeWeight;
        if (lottery < beamWeight) {
            return KingJellyfishAttackTypeEnum.Omnidirectional_Beam;
        }

        lottery -= beamWeight;
        if (lottery < fieldWeight) {
            return KingJellyfishAttackTypeEnum.ElectricField;
        }

        return KingJellyfishAttackTypeEnum.RotatingBeam;
    }

    //=============================================================
    // 攻撃リクエスト処理
    //=============================================================
    internal bool ConsumeAttackRequest() {
        bool requested = attackRequested_;
        attackRequested_ = false;
        return requested;
    }

    //=============================================================
    // 行動ループ処理
    //=============================================================
    internal void ResetActionLoop() {
        currentActionLoopCount_ = 0;
    }

    internal bool ConsumeActionLoop() {
        currentActionLoopCount_++;
        return currentActionLoopCount_ < ActionLoopCount;
    }

    //=============================================================
    // 状態の変更
    //=============================================================
    internal void ChangeState(IKingJellyfishState nextState) {
        if (state_ != null) {
            state_.Exit(this);
        }

        state_ = nextState;
        if (state_ != null) {
            state_.Enter(this);
        }
    }

    //=============================
    // 体当たり攻撃の開始処理
    //=============================
    internal bool BeginChargeAttack() {
        ResolveTarget();
        if (targetEntity_ == null || targetEntity_.transform == null) {
            return false;
        }

        Vector2 start = ToPlane(transform.position);
        Vector2 target = ToPlane(targetEntity_.transform.position);
        Vector2 direction = (target - start).Normalized();
        if (direction.LengthSq() <= 0.001f) {
            direction = Vector2.down;
        }

        chargeStartPosition_ = start;
        chargeTargetPosition_ = target + direction * chargeSettings_.passThroughDistance;
        chargeMoveDirection_ = direction;
        chargeRecoveryVelocity_ = direction * (ChargeSpeed * ChargeInertiaRate);
        chargeDamageFieldDeployed_ = false;
        movementDepth_ = transform.position.z;
        RotateTowardPosition(chargeTargetPosition_);
        SetWeakPointCollisionEnabled(false);
        return true;
    }

    //=============================
    // 体当たり攻撃の予備動作処理
    //=============================
    internal void UpdateChargeTell() {
        // 設定されていない場合、ターゲットの設定
        ResolveTarget();

        // ターゲットの位置に向かって回転する
        if (targetEntity_ != null && targetEntity_.transform != null) {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));
        }

        UpdateAttackTellColor(KingJellyfishAttackTypeEnum.ChargeAttack);
    }

    internal bool UpdateChargeAttack() {
        SetAttackColor(KingJellyfishAttackTypeEnum.ChargeAttack);

        // ターゲットの位置に向かって移動する
        Vector2 currentPosition = ToPlane(transform.position);
        Vector2 toTarget = chargeTargetPosition_ - currentPosition;
        float remainingDistance = toTarget.Length();

        // 目標位置に到達したかどうかを判定する
        if (remainingDistance <= 0.001f) {
            SetPlanePosition(chargeTargetPosition_);
            DeployChargeDamageField();
            return true;
        }

        // 移動距離を計算し、目標位置に到達するかどうかを判定する
        float moveDistance = ChargeSpeed * Time.deltaTime;
        if (moveDistance >= remainingDistance) {
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
    internal void UpdateChargeRecovery(float elapsed) {
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
    internal void DeployChargeDamageField() {
        if (chargeDamageFieldDeployed_) {
            return;
        }

        // ダメージフィールドを生成する
        chargeDamageFieldDeployed_ = true;
        CreateChargeDamageField(chargeStartPosition_, chargeTargetPosition_);
    }

    private void CreateChargeDamageField(Vector2 start, Vector2 end) {
        if (String.IsNullOrEmpty(chargeSettings_.damageFieldPrefabName)) {
            return;
        }

        Entity damageField = ecsGroup.CreateEntity(chargeSettings_.damageFieldPrefabName);
        if (damageField == null) {
            return;
        }

        // 寿命を管理するルートEntityのUpdateが必ず実行されるようにする。
        damageField.enable = true;
        ConfigureChargeDamageField(damageField, start, end);
    }

    private void ConfigureChargeDamageField(Entity damageField, Vector2 start, Vector2 end) {
        Vector2 move = end - start;
        float length = move.Length();
        if (length <= 0.001f) {
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
        if (damageFieldScript != null) {
            damageFieldScript.Configure(start, end, ChargeDamageFieldWidth, chargeSettings_.damage, ChargeDamageFieldDuration, transform.position.z);
        }

        AttackCollision attackCollision = damageField.GetScript<AttackCollision>();
        if (attackCollision != null) {
            attackCollision.Damage = chargeSettings_.damage;
        }
    }

    //=============================
    // 弧を描く移動処理
    //=============================
    internal void BeginArcMove() {
        movementDepth_ = transform.position.z;
        arcMoveStartPosition_ = ToPlane(transform.position);

        Vector2 moveDirection = SelectArcMoveDirection();
        arcMoveEndPosition_ = arcMoveStartPosition_ + moveDirection * MoveDistance;
        arcMoveLastPosition_ = arcMoveStartPosition_;
        arcMoveLastDirection_ = moveDirection;
        arcMoveInertiaVelocity_ = moveDirection * (MoveSpeed * MoveInertiaRate);
        RotateTowardPosition(arcMoveEndPosition_);
    }

    internal bool UpdateArcMove(float elapsed) {
        float duration = MoveDuration;
        float ratio = Mathf.Clamp01(elapsed / duration);

        Vector2 linePosition = Vector2.Lerp(arcMoveStartPosition_, arcMoveEndPosition_, ratio);
        float arcOffset = Mathf.Sin(ratio * Mathf.PI) * MoveArcHeight;
        Vector2 next = new Vector2(linePosition.x, linePosition.y + arcOffset);

        Vector2 frameMove = next - arcMoveLastPosition_;
        if (frameMove.LengthSq() > 0.001f) {
            arcMoveLastDirection_ = frameMove.Normalized();
            arcMoveInertiaVelocity_ = arcMoveLastDirection_ * (MoveSpeed * MoveInertiaRate);
        }
        arcMoveLastPosition_ = next;

        SetPlanePosition(next);

        RotateTowardPosition(arcMoveEndPosition_);
        return ratio >= 1.0f;
    }

    internal void UpdateArcMoveInertia(float elapsed) {
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
    internal void UpdateLaserTell(KingJellyfishAttackTypeEnum attackType) {
        ResolveTarget();
        if (targetEntity_ != null && targetEntity_.transform != null) {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));
        }

        UpdateAttackTellColor(attackType);
    }

    //=============================
    // 8方向レーザー攻撃の発射処理
    //=============================
    internal void FireOmnidirectionalLaser() {
        SetAttackColor(KingJellyfishAttackTypeEnum.Omnidirectional_Beam);

        if (String.IsNullOrEmpty(beamSettings_.laserPrefabName)) {
            return;
        }

        Vector2 origin = ToPlane(transform.position);
        int count = LaserCount;

        // レーザーを全方向に発射する
        for (int i = 0; i < count; i++) {
            float angle = Mathf.PI * 2.0f * i / count;
            Vector2 direction = new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));

            // レーザーエンティティを生成する
            Entity laser = ecsGroup.CreateEntity(beamSettings_.laserPrefabName);
            if (laser == null) {
                continue;
            }

            ConfigureLaserEntity(laser, origin, direction, LaserLength, LaserWidth, beamSettings_.damage, LaserFireDuration, 0.0f);
        }
    }

    //=============================
    // 回転ビーム攻撃
    //=============================
    internal void FireRotatingLasers() {
        SetAttackColor(KingJellyfishAttackTypeEnum.RotatingBeam);

        if (String.IsNullOrEmpty(rotatingBeamSettings_.laserPrefabName)) {
            return;
        }

        Vector2 origin = ToPlane(transform.position);
        int count = RotatingLaserCount;
        for (int i = 0; i < count; i++) {
            float angle = Mathf.PI * 2.0f * i / count;
            Vector2 direction = new Vector2(Mathf.Cos(angle), Mathf.Sin(angle));
            Entity laser = ecsGroup.CreateEntity(rotatingBeamSettings_.laserPrefabName);
            if (laser == null) {
                continue;
            }

            ConfigureLaserEntity(
                laser,
                origin,
                direction,
                RotatingLaserLength,
                RotatingLaserWidth,
                rotatingBeamSettings_.damage,
                RotatingLaserDuration,
                rotatingBeamSettings_.rotationSpeed);
        }
    }

    //=============================
    // 帯電フィールド攻撃
    //=============================
    internal void DeployElectricFields() {
        SetAttackColor(KingJellyfishAttackTypeEnum.ElectricField);

        if (String.IsNullOrEmpty(electricFieldSettings_.fieldPrefabName)) {
            return;
        }

        ResolveTarget();
        Vector2 center = targetEntity_ != null && targetEntity_.transform != null
            ? ToPlane(targetEntity_.transform.position)
            : ToPlane(transform.position);

        for (int i = 0; i < ElectricFieldCount; i++) {
            Vector2 position = center;
            if (i > 0) {
                float angle = RandomUtil.NextFloat() * Mathf.PI * 2.0f;
                float distance = RandomUtil.NextFloat() * electricFieldSettings_.spreadRadius;
                position += new Vector2(Mathf.Cos(angle), Mathf.Sin(angle)) * distance;
            }

            Entity field = ecsGroup.CreateEntity(electricFieldSettings_.fieldPrefabName);
            if (field == null) {
                continue;
            }

            JellyfishElectricField fieldScript = field.GetScript<JellyfishElectricField>();
            if (fieldScript == null) {
                field.Destroy();
                continue;
            }

            float activationDelay = ElectricFieldTellDuration + ElectricFieldSpawnInterval * i;
            fieldScript.Configure(
                position,
                ElectricFieldRadius,
                electricFieldSettings_.damage,
                activationDelay,
                ElectricFieldActiveDuration,
                transform.position.z);
        }
    }

    internal void UpdateLaserRecovery() {
        ResolveTarget();
        if (targetEntity_ != null && targetEntity_.transform != null) {
            RotateTowardPosition(ToPlane(targetEntity_.transform.position));
        }
    }

    internal void UpdateAttackTellColor(KingJellyfishAttackTypeEnum attackType) {
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer == null) {
            return;
        }

        Vector4 color = GetAttackColor(attackType);
        float blinkPhase = (Mathf.Sin(Time.time * Mathf.PI * 10.0f) + 1.0f) * 0.5f;
        float brightness = 0.35f + blinkPhase * 0.65f;
        renderer.color = new Vector4(
            color.x * brightness,
            color.y * brightness,
            color.z * brightness,
            1.0f);
    }

    internal void SetAttackColor(KingJellyfishAttackTypeEnum attackType) {
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer != null) {
            renderer.color = GetAttackColor(attackType);
        }
    }

    internal void EnsureSpriteOpaque() {
        SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
        if (renderer == null) {
            return;
        }

        Vector4 color = renderer.color;
        renderer.color = new Vector4(color.x, color.y, color.z, 1.0f);
    }

    private static Vector4 GetAttackColor(KingJellyfishAttackTypeEnum attackType) {
        if (attackType == KingJellyfishAttackTypeEnum.ChargeAttack) {
            return new Vector4(1.0f, 0.15f, 0.1f, 1.0f);
        }
        if (attackType == KingJellyfishAttackTypeEnum.Omnidirectional_Beam) {
            return new Vector4(0.15f, 1.0f, 0.25f, 1.0f);
        }
        if (attackType == KingJellyfishAttackTypeEnum.ElectricField) {
            return new Vector4(0.15f, 0.55f, 1.0f, 1.0f);
        }
        return new Vector4(0.85f, 0.2f, 1.0f, 1.0f);
    }

    private void ConfigureLaserEntity(
        Entity laser,
        Vector2 origin,
        Vector2 direction,
        float length,
        float width,
        float damage,
        float duration,
        float rotationSpeed) {
        Vector2 normalized = direction.Normalized();
        if (normalized.LengthSq() <= 0.001f) {
            normalized = Vector2.up;
        }

        JellyfishLaser laserScript = laser.GetScript<JellyfishLaser>();
        if (laserScript == null) {
            laser.Destroy();
            return;
        }

        laserScript.Configure(origin, normalized, length, width, damage, duration, transform.position.z, rotationSpeed);
    }


    private void SetPlanePosition(Vector2 position) {
        transform.position = new Vector3(position.x, position.y, movementDepth_);
    }
    private static Vector2 ToPlane(Vector3 position) {
        return new Vector2(position.x, position.y);
    }

    //============================
    // ターゲットの位置に向かって回転する処理
    //============================
    private void RotateTowardPosition(Vector2 targetPosition) {
        Vector2 direction = targetPosition - ToPlane(transform.position);
        if (direction.Length() <= 0.001f) {
            return;
        }

        Vector2 normalized = direction.Normalized();
        float angle = Mathf.Atan2(normalized.x, normalized.y);
        transform.rotation = Quaternion.MakeFromAxis(Vector3.back, angle);
    }

    internal float ChargeTellDuration {
        get { return Positive(chargeSettings_.tellDuration); }
    }

    internal float ChargeMoveDuration {
        get { return Positive(chargeSettings_.moveDuration); }
    }

    internal float ChargeSpeed {
        get { return chargeSettings_.speed > 0.0f ? chargeSettings_.speed : 1.0f; }
    }

    internal float ChargeInertiaRate {
        get { return NonNegative(chargeSettings_.inertiaRate); }
    }

    internal float ChargeRecoveryDuration {
        get { return Positive(chargeSettings_.recoveryDuration); }
    }

    internal float ChargeDamageFieldWidth {
        get { return chargeSettings_.damageFieldWidth > 0.0f ? chargeSettings_.damageFieldWidth : 1.0f; }
    }

    internal float ChargeDamageFieldDuration {
        get { return chargeSettings_.damageFieldDuration > 0.0f ? chargeSettings_.damageFieldDuration : ChargeMoveDuration; }
    }

    internal float IdleDuration {
        get { return idleDuration > 0 ? idleDuration : 0.01f; }
    }

    internal int ActionLoopCount {
        get { return moveSettings_.actionLoopCount > 0 ? moveSettings_.actionLoopCount : 1; }
    }

    internal float MoveDuration {
        get { return Positive(moveSettings_.moveDuration); }
    }

    internal float MoveDistance {
        get { return moveSettings_.moveDistance > 0.0f ? moveSettings_.moveDistance : 1.0f; }
    }

    internal float MoveSpeed {
        get { return moveSettings_.moveSpeed > 0.0f ? moveSettings_.moveSpeed : 1.0f; }
    }

    internal float MoveArcHeight {
        get { return moveSettings_.moveArcHeight; }
    }

    internal float MoveInertiaDuration {
        get { return Positive(moveSettings_.moveInertiaDuration); }
    }

    internal float MoveInertiaRate {
        get { return NonNegative(moveSettings_.moveInertiaRate); }
    }

    internal float LaserTellDuration {
        get { return Positive(beamSettings_.tellDuration); }
    }

    internal float LaserFireDuration {
        get { return Positive(beamSettings_.fireDuration); }
    }

    internal float LaserRecoveryDuration {
        get { return Positive(beamSettings_.recoveryDuration); }
    }

    internal float LaserLength {
        get { return beamSettings_.length > 0.0f ? beamSettings_.length : 1.0f; }
    }

    internal float LaserWidth {
        get { return beamSettings_.width > 0.0f ? beamSettings_.width : 1.0f; }
    }

    internal int LaserCount {
        get { return beamSettings_.laserCount > 0 ? beamSettings_.laserCount : 8; }
    }

    internal int ElectricFieldCount {
        get { return electricFieldSettings_.fieldCount > 0 ? electricFieldSettings_.fieldCount : 1; }
    }

    internal float ElectricFieldTellDuration {
        get { return Positive(electricFieldSettings_.tellDuration); }
    }

    internal float ElectricFieldSpawnInterval {
        get { return NonNegative(electricFieldSettings_.spawnInterval); }
    }

    internal float ElectricFieldActiveDuration {
        get { return Positive(electricFieldSettings_.activeDuration); }
    }

    internal float ElectricFieldRecoveryDuration {
        get { return Positive(electricFieldSettings_.recoveryDuration); }
    }

    internal float ElectricFieldRadius {
        get { return electricFieldSettings_.radius > 0.0f ? electricFieldSettings_.radius : 1.0f; }
    }

    internal int RotatingLaserCount {
        get { return rotatingBeamSettings_.laserCount > 0 ? rotatingBeamSettings_.laserCount : 1; }
    }

    internal float RotatingLaserTellDuration {
        get { return Positive(rotatingBeamSettings_.tellDuration); }
    }

    internal float RotatingLaserDuration {
        get { return Positive(rotatingBeamSettings_.duration); }
    }

    internal float RotatingLaserRecoveryDuration {
        get { return Positive(rotatingBeamSettings_.recoveryDuration); }
    }

    internal float RotatingLaserLength {
        get { return rotatingBeamSettings_.length > 0.0f ? rotatingBeamSettings_.length : 1.0f; }
    }

    internal float RotatingLaserWidth {
        get { return rotatingBeamSettings_.width > 0.0f ? rotatingBeamSettings_.width : 1.0f; }
    }

    private Vector2 SelectArcMoveDirection() {
        if (moveSettings_.moveTowardTarget) {
            ResolveTarget();
            if (targetEntity_ != null && targetEntity_.transform != null) {
                Vector2 toTarget = ToPlane(targetEntity_.transform.position) - ToPlane(transform.position);
                if (toTarget.LengthSq() > 0.001f) {
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
    private void ResolveTarget() {
        if (targetEntity_ == null && !String.IsNullOrEmpty(targetEntityName)) {
            targetEntity_ = ecsGroup.FindEntity(targetEntityName);
        }
    }

    private T GetOrAddSettings<T>() where T : MonoScript {
        T settings = entity.GetScript<T>();
        return settings != null ? settings : entity.AddScript<T>();
    }

    private static float Positive(float value) {
        return value > 0.0f ? value : 0.01f;
    }

    private static float NonNegative(float value) {
        return value > 0.0f ? value : 0.0f;
    }
}
