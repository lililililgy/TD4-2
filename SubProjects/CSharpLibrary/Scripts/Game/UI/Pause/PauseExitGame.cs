using System;
using System.Collections.Generic;

public class PauseExitGame : PauseCommon {
	void OnSelect() {
		isSelect = true;
	}

	void OnDeselect() {
		isSelect = false;
	}

	void OnSubmit() {
		Application.Quit();
	}

}
