using System;
using System.Collections.Generic;

public class ClearRestart : BaseUI {

	void OnSelect() {
		isSelect = true;
	}

	void OnDeselect() {
		isSelect = false;
	}

	void OnSubmit() {
		SceneManager.LoadScene("GameScene");
	}

}
