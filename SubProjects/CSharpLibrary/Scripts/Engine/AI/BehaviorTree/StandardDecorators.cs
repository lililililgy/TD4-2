using System;

/// <summary>
/// ノードの実行完了後にクールダウン時間（待機時間）を設け、連続して実行されるのを制限するデコレーター。
/// 例：ボスの強力な攻撃を15秒に1回しか打てないようにする等の用途に使用。
/// </summary>
public class CooldownDecorator : BehaviorDecorator
{
    /// <summary>
    /// 再実行可能になるまでのクールダウン時間（秒）。
    /// </summary>
    public float cooldownTime = 5.0f;

    /// <summary>
    /// ノード実行前に、前回実行時から指定時間が経過しているかを判定する。
    /// </summary>
    public override bool CalculateCondition(Blackboard blackboard, Entity owner)
    {
        // ノード固有のハッシュを利用して、Blackboardに実行時刻を保存するキーを生成
        uint key = BehaviorTreeLoader.HashString("LastExecution_" + NodeIdHash);
        float lastTime = blackboard.GetFloat(key, -100.0f); // 初期値は十分に古い時間
        
        // 経過時間がクールダウン時間を超えていれば実行を許可
        if (Time.time - lastTime >= cooldownTime)
        {
            // 実行許可と同時に現在の時刻を記録し、次のクールダウンを開始する
            // ※本来はノードの実行が完了（Success）したタイミングで記録するのが厳密なUEの挙動だが、
            // 現在のアーキテクチャでは開始時にセットする簡易仕様としている。
            blackboard.SetFloat(key, Time.time);
            return true;
        }
        
        // クールダウン中の場合は実行失敗とする
        return false;
    }
}

/// <summary>
/// アタッチされたノードを、指定した回数だけ強制的に繰り返し実行させるデコレーター。
/// </summary>
public class LoopDecorator : BehaviorDecorator
{
    /// <summary>
    /// ループさせる回数。
    /// </summary>
    public int loopCount = 3;
    
    /// <summary>
    /// trueにすると回数を無視して無限にループさせる。
    /// </summary>
    public bool infinite = false;

    // ループ自体は常に実行を許可するため、事前条件は無条件でtrue
    public override bool CalculateCondition(Blackboard blackboard, Entity owner) => true;

    /// <summary>
    /// ノード本体が処理を終えた（SuccessまたはFailure）際に、結果を加工して強制的に Running に戻す。
    /// </summary>
    public override NodeStatus PostProcessStatus(NodeStatus currentStatus, Blackboard blackboard)
    {
        // まだ実行中の場合はそのまま返す
        if (currentStatus == NodeStatus.Running) return NodeStatus.Running;

        // 成功または失敗した場合、ループのカウントを進める
        uint key = BehaviorTreeLoader.HashString("LoopIdx_" + NodeIdHash);
        int currentIdx = blackboard.GetInt(key, 0);

        // 無限ループなら常にRunningを返して親のSequenceを足止めし、次フレームに再挑戦させる
        if (infinite) return NodeStatus.Running; 

        currentIdx++;
        
        // まだ指定回数に達していない場合はカウントを保存し、Runningを返して継続させる
        if (currentIdx < loopCount)
        {
            blackboard.SetInt(key, currentIdx);
            return NodeStatus.Running; 
        }

        // 指定回数ループし終えたらカウントをリセットし、ノード本来の結果（Success/Failure）を親に返す
        blackboard.SetInt(key, 0);
        return currentStatus;
    }
}

/// <summary>
/// アタッチされたノードが内部で Failure（失敗） を返したとしても、
/// 親ノード（Sequenceなど）に対して強制的に Success（成功） として報告するデコレーター。
/// 「失敗してもツリー全体の進行を止めない」ためのフロー制御（オプショナルな行動など）に使用される。
/// </summary>
public class ForceSuccessDecorator : BehaviorDecorator
{
    // 実行自体は常に許可する
    public override bool CalculateCondition(Blackboard blackboard, Entity owner) => true;

    /// <summary>
    /// 実行完了後に結果をインターセプトし、Failure の場合でも Success に上書きする。
    /// </summary>
    public override NodeStatus PostProcessStatus(NodeStatus currentStatus, Blackboard blackboard)
    {
        // 実行中の場合はそのまま返す（途中で強制終了させるわけではない）
        if (currentStatus == NodeStatus.Running) return NodeStatus.Running;
        
        // FailureでもSuccessでも、最終的には必ずSuccessを返す
        return NodeStatus.Success;
    }
}
