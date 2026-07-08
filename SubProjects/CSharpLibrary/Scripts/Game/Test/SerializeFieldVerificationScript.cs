using System;
using System.Collections.Generic;

public enum TestEnum {
	A,
	B,
	C
}

public struct SerializeFieldTestStruct {
	[SerializeField] public int intVal;
	[SerializeField] public float floatVal;
}

public class SerializeFieldTestClass {
	[SerializeField] public int intVal;
	[SerializeField] public string strVal;
}

public class SerializeFieldVerificationScript : MonoScript {
	[SerializeField] int valInt = 0;
	[SerializeField] float valFloat = 0f;
	[SerializeField] double valDouble = 0.0;
	[SerializeField] bool valBool = false;
	[SerializeField] string valString = "";
	[SerializeField] TestEnum valEnum = TestEnum.A;
	[SerializeField] Vector2 valVec2 = Vector2.zero;
	[SerializeField] Vector3 valVec3 = Vector3.zero;
	[SerializeField] Vector4 valVec4 = Vector4.zero;

	[SerializeField] List<int> valListInt = null;
	[SerializeField] List<float> valListFloat = null;
	[SerializeField] List<bool> valListBool = null;
	[SerializeField] List<string> valListString = null;
	[SerializeField] List<Vector3> valListVec3 = null;
	[SerializeField] List<TestEnum> valListEnum = null;

	[SerializeField] SerializeFieldTestStruct valStruct;
	[SerializeField] List<SerializeFieldTestStruct> valListStruct = null;

	[SerializeField] SerializeFieldTestClass valClass = null;
	[SerializeField] List<SerializeFieldTestClass> valListClass = null;

	// 空リストの検証用変数
	[SerializeField] List<int> emptyListInt = null;
	[SerializeField] List<SerializeFieldTestStruct> emptyListStruct = null;
	[SerializeField] List<SerializeFieldTestClass> emptyListClass = null;

	public override void Initialize() {
		if (valInt != 42) throw new Exception("valInt assertion failed: " + valInt);
		if (Math.Abs(valFloat - 3.14f) > 0.001f) throw new Exception("valFloat assertion failed: " + valFloat);
		if (Math.Abs(valDouble - 2.71828) > 0.00001) throw new Exception("valDouble assertion failed: " + valDouble);
		if (!valBool) throw new Exception("valBool assertion failed: " + valBool);
		if (valString != "hello serializefield") throw new Exception("valString assertion failed: " + valString);
		if (valEnum != TestEnum.C) throw new Exception("valEnum assertion failed: " + valEnum);

		if (Math.Abs(valVec2.x - 1.5f) > 0.001f || Math.Abs(valVec2.y - 2.5f) > 0.001f)
			throw new Exception("valVec2 assertion failed: " + valVec2.x + ", " + valVec2.y);
		if (Math.Abs(valVec3.x - 3.5f) > 0.001f || Math.Abs(valVec3.y - 4.5f) > 0.001f || Math.Abs(valVec3.z - 5.5f) > 0.001f)
			throw new Exception("valVec3 assertion failed: " + valVec3.x + ", " + valVec3.y + ", " + valVec3.z);
		if (Math.Abs(valVec4.x - 6.5f) > 0.001f || Math.Abs(valVec4.y - 7.5f) > 0.001f || Math.Abs(valVec4.z - 8.5f) > 0.001f || Math.Abs(valVec4.w - 9.5f) > 0.001f)
			throw new Exception("valVec4 assertion failed: " + valVec4.x + ", " + valVec4.y + ", " + valVec4.z + ", " + valVec4.w);

		if (valListInt == null || valListInt.Count != 2 || valListInt[0] != 10 || valListInt[1] != 20)
			throw new Exception("valListInt assertion failed");
		if (valListFloat == null || valListFloat.Count != 2 || Math.Abs(valListFloat[0] - 1.1f) > 0.001f || Math.Abs(valListFloat[1] - 2.2f) > 0.001f)
			throw new Exception("valListFloat assertion failed");
		if (valListBool == null || valListBool.Count != 2 || !valListBool[0] || valListBool[1])
			throw new Exception("valListBool assertion failed");
		if (valListString == null || valListString.Count != 2 || valListString[0] != "one" || valListString[1] != "two")
			throw new Exception("valListString assertion failed");
		if (valListVec3 == null || valListVec3.Count != 2 ||
			Math.Abs(valListVec3[0].x - 1f) > 0.001f || Math.Abs(valListVec3[0].y - 2f) > 0.001f || Math.Abs(valListVec3[0].z - 3f) > 0.001f ||
			Math.Abs(valListVec3[1].x - 4f) > 0.001f || Math.Abs(valListVec3[1].y - 5f) > 0.001f || Math.Abs(valListVec3[1].z - 6f) > 0.001f)
			throw new Exception("valListVec3 assertion failed");
		if (valListEnum == null || valListEnum.Count != 2 || valListEnum[0] != TestEnum.B || valListEnum[1] != TestEnum.C)
			throw new Exception("valListEnum assertion failed");

		if (valStruct.intVal != 99 || Math.Abs(valStruct.floatVal - 4.5f) > 0.001f)
			throw new Exception("valStruct assertion failed: " + valStruct.intVal + ", " + valStruct.floatVal);

		if (valListStruct == null || valListStruct.Count != 2 ||
			valListStruct[0].intVal != 101 || Math.Abs(valListStruct[0].floatVal - 1.5f) > 0.001f ||
			valListStruct[1].intVal != 102 || Math.Abs(valListStruct[1].floatVal - 2.5f) > 0.001f)
			throw new Exception("valListStruct assertion failed");

		if (valClass == null || valClass.intVal != 999 || valClass.strVal != "nested class")
			throw new Exception("valClass assertion failed");

		if (valListClass == null || valListClass.Count != 2 ||
			valListClass[0] == null || valListClass[0].intVal != 888 || valListClass[0].strVal != "elem1" ||
			valListClass[1] == null || valListClass[1].intVal != 777 || valListClass[1].strVal != "elem2")
			throw new Exception("valListClass assertion failed");

		// 空リストの検証
		if (emptyListInt == null || emptyListInt.Count != 0)
			throw new Exception("emptyListInt assertion failed: should be empty (Count == 0)");
		if (emptyListStruct == null || emptyListStruct.Count != 0)
			throw new Exception("emptyListStruct assertion failed: should be empty (Count == 0)");
		if (emptyListClass == null || emptyListClass.Count != 0)
			throw new Exception("emptyListClass assertion failed: should be empty (Count == 0)");

		Debug.Log("SerializeFieldVerificationScript: All SerializeField assertions (including empty lists) passed successfully!");
	}
}
