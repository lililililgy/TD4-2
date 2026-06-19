using System;

/// <summary>
/// ノードがデコレーター（条件ノード）であることを示す属性。
/// エディタ上で青色のヘッダーで表示される。
/// </summary>
[AttributeUsage(AttributeTargets.Class)]
public class DecoratorAttribute : Attribute
{
}
