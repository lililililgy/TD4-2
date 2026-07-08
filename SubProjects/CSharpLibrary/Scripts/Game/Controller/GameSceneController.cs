using System;
using System.Collections.Generic;



public class GameSceneController : MonoScript {

	public override void Initialize() {

	}

	public override void Update() {
		if (CheckInput()) {
			SceneManager.Add("PauseScene");
		}
	}



	private bool CheckInput() {
		if(Input.TriggerKey(KeyCode.Escape)) {
			return true;
		}

		if(Input.TriggerGamepad(Gamepad.Start)) {
			return true;
		}

		return false;
	}


}
