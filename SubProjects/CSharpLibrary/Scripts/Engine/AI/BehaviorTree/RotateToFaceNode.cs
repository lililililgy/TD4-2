using System;

/// <summary>
/// Blackboard上のターゲット（Entity）を向き続ける、あるいは瞬間的に向くアクションノード。
/// 攻撃開始前のエイムや、追従時の方向転換に使用する。
/// </summary>
public class RotateToFaceNode : BehaviorNode
{
    /// <summary>
    /// 向き先となるターゲットが格納されているBlackboardのキー。
    /// Entityオブジェクトが格納されていることを期待する。
    /// </summary>
    [BlackboardKey]
    public string targetKey = "Target";

    /// <summary>
    /// 回転速度（度/秒）。0を指定すると即座に向く。
    /// </summary>
    public float rotationSpeed = 360.0f;

    /// <summary>
    /// 目標方向との角度差がこれ以下になればSuccessを返す閾値。
    /// </summary>
    public float precisionAngle = 5.0f;

    protected override NodeStatus Execute(Blackboard blackboard, Entity owner)
    {
        uint key = BehaviorTreeLoader.HashString(targetKey);
        Entity target = blackboard.GetEntity(key);

        if (target == null)
        {
            return NodeStatus.Failure;
        }

        var intent = owner.GetComponent<AgentIntentComponent>();
        if (intent == null)
        {
            return NodeStatus.Failure;
        }

        Vector3 ownerPos = owner.transform.position;
        Vector3 targetPos = target.transform.position;

        // 水平面（XZ）での向きを計算
        Vector3 diff = targetPos - ownerPos;
        diff.y = 0;

        if (diff.Length() < 0.01f)
        {
            intent.useDesiredRotation = false;
            return NodeStatus.Success;
        }

        // 目標とする回転を計算
        Quaternion targetRot = Quaternion.LookRotation(diff.Normalized(), Vector3.up);
        
        // Intentに意図を書き込む（実際の回転処理はC++側のMovementSystem等が行う）
        intent.desiredRotation = targetRot;
        intent.useDesiredRotation = true;

        // 角度差をチェックして終了判定
        float angle = Quaternion.Angle(owner.transform.rotation, targetRot);
        if (angle <= precisionAngle)
        {
            // 目標角度に到達したら、Intentの制御をオフにしてSuccessを返す
            // （Successを返した後も向き続けたい場合は、このノードをループさせるか
            //   別のノードで制御を維持する設計にする）
            intent.useDesiredRotation = false;
            return NodeStatus.Success;
        }

        return NodeStatus.Running;
    }

    public override void OnAbort(Blackboard blackboard, Entity owner)
    {
        // 中断時は回転制御をオフにする
        var intent = owner.GetComponent<AgentIntentComponent>();
        if (intent != null)
        {
            intent.useDesiredRotation = false;
        }
    }
}

