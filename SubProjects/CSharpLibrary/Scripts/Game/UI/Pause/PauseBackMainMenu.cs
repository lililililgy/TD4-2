using System;
using System.Collections.Generic;

public class PauseBackMainMenu : PauseCommon {
	void OnSelect() {
		isSelect = true;
	}

	void OnDeselect() {
		isSelect = false;
	}

	void OnSubmit() {
		SceneManager.LoadScene("TitleScene");
	}

}
