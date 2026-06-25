using System;
using System.Collections.Generic;

public class UITest4 : MonoScript {

	[SerializeField] string nextSceneName = "GameScene";


	public void OnSelect() {
		if (nextSceneName != "") {
			SceneManager.LoadScene(nextSceneName);
		}
	}

}
