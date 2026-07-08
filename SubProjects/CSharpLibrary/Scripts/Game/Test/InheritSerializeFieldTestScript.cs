using System;

public class ParentTestScript : MonoScript {
	[SerializeField] float parentValue = 10f;

	public float GetParentValue() {
		return parentValue;
	}
}

public class ChildTestScript : ParentTestScript {
	public override void Initialize() {
		float val = GetParentValue();
		if (Math.Abs(val - 25.0f) > 0.001f) {
			Debug.LogError("InheritSerializeFieldTest Failed: parentValue is " + val + ", expected 25.0");
			throw new Exception("InheritSerializeFieldTest Assertion Failed: parentValue was not loaded correctly.");
		} else {
			Debug.Log("InheritSerializeFieldTest Passed: parentValue is correctly 25.0");
		}
	}
}
