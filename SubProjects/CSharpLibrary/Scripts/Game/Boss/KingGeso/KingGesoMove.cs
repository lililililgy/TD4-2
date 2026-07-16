using System;
using System.Collections.Generic;

//============================================================
// Loosely follows the player while preserving the boss depth.
//============================================================
public class KingGesoMove : MonoScript {

    [SerializeField] public string targetEntityName = "Player";
    [SerializeField] public Vector2 followOffset = Vector2.zero;
    [SerializeField] public float maxSpeed = 120.0f;
    [SerializeField] public float response = 1.25f;
    [SerializeField] public float stopDistance = 500.0f;
    [SerializeField] public float slowDownDistance = 600.0f;

    private Entity target_;
    private HP hp_;
    private Vector2 velocity_;
    private float movementDepth_;

    public override void Initialize()
    {
        target_ = FindTarget();
        hp_ = entity.GetScript<HP>();
        velocity_ = Vector2.zero;
        movementDepth_ = transform.position.z;
    }

    public override void Update()
    {
        if (hp_ != null && hp_.IsDead)
        {
            velocity_ = Vector2.zero;
            return;
        }

        if (!IsTargetValid())
        {
            target_ = FindTarget();
            if (target_ == null)
            {
                SlowToStop();
                return;
            }
        }

        Vector3 currentPosition = transform.position;
        Vector3 targetPosition = target_.transform.position;
        Vector2 current = new Vector2(currentPosition.x, currentPosition.y);
        Vector2 target = new Vector2(targetPosition.x, targetPosition.y) + followOffset;
        Vector2 toTarget = target - current;
        float distance = toTarget.Length();

        Vector2 desiredVelocity = Vector2.zero;
        float safeStopDistance = stopDistance > 0.0f ? stopDistance : 0.0f;
        if (distance > safeStopDistance && distance > 0.001f)
        {
            float remainingDistance = distance - safeStopDistance;
            float safeSlowDownDistance = slowDownDistance > 0.0f ? slowDownDistance : 0.01f;
            float speedRatio = Mathf.Clamp01(remainingDistance / safeSlowDownDistance);
            float safeMaxSpeed = maxSpeed > 0.0f ? maxSpeed : 0.0f;
            desiredVelocity = toTarget.Normalized() * (safeMaxSpeed * speedRatio);
        }

        float blend = Mathf.Clamp01((response > 0.0f ? response : 0.01f) * Time.deltaTime);
        velocity_ = Vector2.Lerp(velocity_, desiredVelocity, blend);
        Vector2 next = current + velocity_ * Time.deltaTime;
        transform.position = new Vector3(next.x, next.y, movementDepth_);
    }

    private void SlowToStop()
    {
        float blend = Mathf.Clamp01((response > 0.0f ? response : 0.01f) * Time.deltaTime);
        velocity_ = Vector2.Lerp(velocity_, Vector2.zero, blend);

        Vector3 current = transform.position;
        Vector2 next = new Vector2(current.x, current.y) + velocity_ * Time.deltaTime;
        transform.position = new Vector3(next.x, next.y, movementDepth_);
    }

    private bool IsTargetValid()
    {
        return target_ != null && target_.Id > 0 && target_.transform != null;
    }

    private Entity FindTarget()
    {
        if (System.String.IsNullOrEmpty(targetEntityName))
        {
            return null;
        }

        return ecsGroup.FindEntity(targetEntityName);
    }
}
