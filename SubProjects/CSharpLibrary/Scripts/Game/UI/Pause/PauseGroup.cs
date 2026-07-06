using System;
using System.Collections.Generic;

public class PauseGroup : MonoScript {
	
	/// <summary>
	///  pause sceneにおいてキャンセルボタンが押されたとき
	/// </summary>
	public void OnCancel() {
		SceneManager.Unload("PauseScene");
	}

	public void OnUISelect(string selectElementName) {
		SEPlayer audio = entity.GetComponent<SEPlayer>();
		if(audio) {
			audio.Play();
		}
	}

}
