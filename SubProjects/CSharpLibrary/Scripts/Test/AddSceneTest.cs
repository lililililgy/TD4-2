using System;
using System.Collections.Generic;

public class AddSceneTest : MonoScript {
	public override void Initialize() {
		
	}

	public override void Update() {
		if(Input.TriggerKey(KeyCode.Space)) {
			SceneManager.Add("TestScene");
		}
	}
}
