using System;
using System.Collections.Generic;

public class TestSceneScript : MonoScript {
	public override void Initialize() {
		
	}

	public override void Update() {
		if (Input.TriggerKey(KeyCode.T)) {
			SceneManager.Add("TitleScene");
		}

		if (Input.TriggerKey(KeyCode.Y)) {
			SceneManager.Unload("TitleScene");
		}

	}
}
