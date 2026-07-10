using System;
using System.Collections.Generic;

public class PauseStartCheckPoint : PauseCommon {
	void OnSelect() {
		isSelect = true;
	}

	void OnDeselect() {
		isSelect = false;
	}
}
