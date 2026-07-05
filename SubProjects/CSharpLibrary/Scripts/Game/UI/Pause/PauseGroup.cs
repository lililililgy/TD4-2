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
		AudioSource audio = entity.GetComponent<AudioSource>();
		if(audio) {
			audio.Play();
		}
	}

}
