using System;

/// <summary>
/// 論理ゲート（AND, OR, NOT）の種類を定義する列挙型。
/// </summary>
public enum LogicOp
{
    /// <summary>AかつB（両方がTrueならTrue）</summary>
    And,
    /// <summary>AまたはB（どちらか一方がTrueならTrue）</summary>
    Or,
    /// <summary>Aではない（AがFalseならTrue）</summary>
    Not
}

/// <summary>
/// Blackboardの複数の変数（条件）を組み合わせて、複雑な論理判定を行うデコレーター。
/// 例：「プレイヤーが視界内にいる AND 怒り状態である」などの複合条件をエディタ上で作成できる。
/// </summary>
public class LogicDecorator : BehaviorDecorator
{
    /// <summary>
    /// 適用する論理演算子の種類。
    /// </summary>
    public LogicOp operation = LogicOp.And;

    /// <summary>
    /// 評価対象となる1つ目のBlackboardキー名。
    /// </summary>
    [BlackboardKey]
    public string keyA = "";

    /// <summary>
    /// 評価対象となる2つ目のBlackboardキー名。
    /// operation が Not の場合は無視される。
    /// </summary>
    [BlackboardKey]
    public string keyB = "";

    /// <summary>
    /// 指定された論理演算子に従って、キーAとキーBの値を評価する。
    /// </summary>
    public override bool CalculateCondition(Blackboard blackboard, Entity owner)
    {
        uint hashA = BehaviorTreeLoader.HashString(keyA);
        uint hashB = BehaviorTreeLoader.HashString(keyB);

        bool valA = IsTrue(blackboard, hashA);
        bool valB = IsTrue(blackboard, hashB);

        switch (operation)
        {
            case LogicOp.And: return valA && valB;
            case LogicOp.Or:  return valA || valB;
            case LogicOp.Not: return !valA; // Notの場合は keyA だけを評価する
            default: return false;
        }
    }

    /// <summary>
    /// 指定したキーの値が「True（条件を満たしている）」かどうかを、型を問わずに判定する内部メソッド。
    /// </summary>
    private bool IsTrue(Blackboard bb, uint key)
    {
        // キーが指定されていない、または存在しない場合はFalse
        if (key == 0 || !bb.HasKey(key)) return false;
        
        object val = bb.GetValueAsObject(key);
        if (val == null) return false;

        // 型に応じた判定
        if (val is bool b) return b;
        if (val is int i) return i != 0; // 0以外ならTrue
        if (val is float f) return f != 0.0f; // 0.0以外ならTrue
        if (val is string s) return !string.IsNullOrEmpty(s); // 空文字でなければTrue
        
        // その他の参照型であれば、インスタンスが存在する（nullでない）ためTrue
        return true;
    }
}
