using System;
using System.Collections.Generic;



public class GameSceneController : MonoScript {


	[SerializeField] List<KeyCode> openPauseKeyInputs = new List<KeyCode>();
	[SerializeField] List<Gamepad> openPausePadInputs = new List<Gamepad>();


	public override void Initialize() {

	}

	public override void Update() {
		if (CheckInput()) {
			SceneManager.Add("PauseScene");
		}
	}



	private bool CheckInput() {
		for (int i = 0; i < openPauseKeyInputs.Count; i++) {
			if (Input.TriggerKey(openPauseKeyInputs[i])) {
				return true;
			}
		}

		for (int i = 0; i < openPausePadInputs.Count; i++) {
			if (Input.TriggerGamepad(openPausePadInputs[i])) {
				return true;
			}
		}

		if(Input.TriggerKey(KeyCode.Escape)) {
			return true;
		}

		return false;
	}


}
