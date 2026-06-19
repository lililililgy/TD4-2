using System;

/// <summary>
/// Blackboardの変数を監視した割り込み（Observer Aborts）のルールを定義する列挙型。
/// </summary>
public enum ObserverAbortPolicy
{
    /// <summary>監視による割り込みを行わない。</summary>
    None,
    /// <summary>実行中に自身の条件が満たされなくなった際、即座に自身（およびその子孫）を中断する。</summary>
    Self,
    /// <summary>他の優先度の低いタスクを実行中に、自身の条件が満たされた際、そのタスクを中断して自身に実行権を移す。</summary>
    LowerPriority,
    /// <summary>SelfとLowerPriorityの両方を適用する。</summary>
    Both
}

/// <summary>
/// Blackboardの値をどのように比較するかを定義する演算子。
/// </summary>
public enum BlackboardQuery
{
    IsSet,          // 値が設定されている（null/0以外）
    IsNotSet,       // 値が設定されていない（null/0）
    Equal,          // 一致する
    NotEqual,       // 一致しない
    Less,           // 未満
    LessOrEqual,    // 以下
    Greater,        // 超過
    GreaterOrEqual, // 以上
    StringContains  // 文字列が含まれる
}

/// <summary>
/// Blackboardの特定の変数（キー）を監視し、その値に基づいて実行の可否を判定するデコレーター。
/// Unreal Engineの Blackboard Decorator に相当する。
/// </summary>
public class BlackboardDecorator : BehaviorDecorator
{
    /// <summary>
    /// 監視対象となるBlackboardのキー名。
    /// </summary>
    [BlackboardKey]
    public string keyName = "";

    /// <summary>
    /// 比較演算子。
    /// </summary>
    public BlackboardQuery queryOperator = BlackboardQuery.IsSet;

    /// <summary>
    /// 比較対象となる値（文字列として保持し、実行時に変換）。
    /// </summary>
    public string compareValue = "";

    /// <summary>
    /// ツリー初期化時に呼ばれ、このデコレーターが監視すべきキーのハッシュ値を BehaviorTree に登録する。
    /// </summary>
    public override uint GetMonitoredKey()
    {
        return BehaviorTreeLoader.HashString(keyName);
    }

    /// <summary>
    /// ノード実行前に呼ばれ、Blackboardの値が条件を満たしているかを判定する。
    /// </summary>
    public override bool CalculateCondition(Blackboard blackboard, Entity owner)
    {
        uint key = BehaviorTreeLoader.HashString(keyName);

        // 1. 基本的な存在チェック (IsSet / IsNotSet)
        bool hasKey = blackboard.HasKey(key);
        if (queryOperator == BlackboardQuery.IsNotSet) return !hasKey;
        if (!hasKey) return false;

        object val = blackboard.GetValueAsObject(key);
        if (val == null) return queryOperator == BlackboardQuery.IsNotSet;
        if (queryOperator == BlackboardQuery.IsSet) 
        {
            if (val is int i) return i != 0;
            if (val is float f) return f != 0.0f;
            if (val is bool b) return b;
            if (val is string s) return !string.IsNullOrEmpty(s);
            return true;
        }

        // 2. 詳細な比較ロジック
        try
        {
            switch (queryOperator)
            {
                case BlackboardQuery.Equal:
                    return string.Equals(val.ToString(), compareValue, StringComparison.OrdinalIgnoreCase);
                case BlackboardQuery.NotEqual:
                    return !string.Equals(val.ToString(), compareValue, StringComparison.OrdinalIgnoreCase);

                case BlackboardQuery.Less:
                case BlackboardQuery.LessOrEqual:
                case BlackboardQuery.Greater:
                case BlackboardQuery.GreaterOrEqual:
                    if (val is int iv) {
                        int cv = int.Parse(compareValue);
                        if (queryOperator == BlackboardQuery.Less) return iv < cv;
                        if (queryOperator == BlackboardQuery.LessOrEqual) return iv <= cv;
                        if (queryOperator == BlackboardQuery.Greater) return iv > cv;
                        if (queryOperator == BlackboardQuery.GreaterOrEqual) return iv >= cv;
                    }
                    if (val is float fv) {
                        float cv = float.Parse(compareValue);
                        if (queryOperator == BlackboardQuery.Less) return fv < cv;
                        if (queryOperator == BlackboardQuery.LessOrEqual) return fv <= cv;
                        if (queryOperator == BlackboardQuery.Greater) return fv > cv;
                        if (queryOperator == BlackboardQuery.GreaterOrEqual) return fv >= cv;
                    }
                    break;

                case BlackboardQuery.StringContains:
                    return val.ToString().Contains(compareValue);
            }
        }
        catch
        {
            // パース失敗時などは安全のために false を返す
            return false;
        }

        return false;
    }
}

