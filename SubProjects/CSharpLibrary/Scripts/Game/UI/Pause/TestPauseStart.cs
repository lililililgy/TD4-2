using System;
using System.Collections.Generic;

public class TestPauseStart : MonoScript {
	public override void Initialize() {

	}

	public override void Update() {
		if (Input.TriggerKey(KeyCode.H)) {
			SceneManager.Add("PauseScene");
		}
	}
}
