using System;
using System.Collections.Generic;


public class Test : MonoScript {


	//[SerializeField] float testFloats = 1f;
	//[SerializeField] int testInt = 1;
	//[SerializeField] bool testBool = false;
	//[SerializeField] string testString = "test";
	[SerializeField] private Vector2 testVector2 = Vector2.zero;
	//[SerializeField] private Vector3 testVector3 = Vector3.zero;
	//[SerializeField] private Vector4 testVector4 = Vector4.zero;

	[SerializeField] Vector4 color = Vector4.red;
	[SerializeField] private Vector2 min = Vector2.zero;

    public override void Initialize() {
		
	}

	public override void Update() {
		if (Input.TriggerKey(KeyCode.Space)) {
			SceneManager.Add("TitleScene");
		}

		if (Input.TriggerKey(KeyCode.P)) {
			SceneManager.SetUpdatePaused("TitleScene", true);
		}

		if (Input.TriggerKey(KeyCode.O)) {
			SceneManager.SetUpdatePaused("TitleScene", false);
		}
	}
}
