using System;
using System.Collections.Generic;

public class PauseGroup : MonoScript {
	
	/// <summary>
	///  pause sceneにおいてキャンセルボタンが押されたとき
	/// </summary>
	public void OnCancel() {
		SceneManager.Unload("PauseScene");
	}

}
