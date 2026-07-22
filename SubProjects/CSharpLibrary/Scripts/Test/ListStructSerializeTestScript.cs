using System;
using System.Collections.Generic;
using ONEngine;

[Serializable]
public struct TestSubStruct
{
    public int id;
    public float val;
}

public class ListStructSerializeTestScript : MonoScript
{
    [SerializeField]
    public List<TestSubStruct> structList = new List<TestSubStruct>
    {
        new TestSubStruct { id = 1, val = 1.5f },
        new TestSubStruct { id = 2, val = 3.0f }
    };

    public override void Update()
    {
    }
}
