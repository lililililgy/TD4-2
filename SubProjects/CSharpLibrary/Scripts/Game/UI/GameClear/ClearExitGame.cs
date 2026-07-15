using System;
using System.Collections.Generic;

public class ClearExitGame : BaseUI {

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
