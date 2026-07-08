using System;
using System.Collections.Generic;

public class PauseResume : PauseCommon {
	void OnSelect() {
		isSelect = true;
	}

	void OnDeselect() {
		isSelect = false;
	}
}
