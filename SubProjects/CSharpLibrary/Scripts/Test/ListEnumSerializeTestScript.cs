using System;
using System.Collections.Generic;
using ONEngine;

public enum TestIntEnum : int
{
    A = 0,
    B = 1,
    C = 2
}

public enum TestByteEnum : byte
{
    X = 10,
    Y = 20,
    Z = 30
}

public class ListEnumSerializeTestScript : MonoScript
{
    [SerializeField]
    public List<TestIntEnum> intEnumList = new List<TestIntEnum> { TestIntEnum.A, TestIntEnum.B, TestIntEnum.C };

    [SerializeField]
    public List<TestByteEnum> byteEnumList = new List<TestByteEnum> { TestByteEnum.X, TestByteEnum.Y };

    public override void Update()
    {
        // ロードに成功していれば中身が設定されているはず
        // 自動テストの検証用
    }
}
