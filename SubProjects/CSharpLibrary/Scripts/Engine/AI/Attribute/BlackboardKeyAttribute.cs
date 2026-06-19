using System;

/// <summary>
/// フィールドがBlackboardのキーであることを示す属性。
/// エディタ上でドロップダウン選択が可能になる。
/// </summary>
[AttributeUsage(AttributeTargets.Field)]
public class BlackboardKeyAttribute : Attribute
{
}
