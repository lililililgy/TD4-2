using System;
using System.Collections.Generic;

public class PauseGroup : MonoScript
{

	[SerializeField] public float time = 0f;
	[SerializeField] public float speed = 1.2f;
	[SerializeField] public float height = 0.1f;
	[SerializeField] public Vector4 defaultColor = Vector4.one;
	[SerializeField] public Vector4 selectedColor = Vector4.red;

	/// <summary>
	///  pause sceneにおいてキャンセルボタンが押されたとき
	/// </summary>
	public void OnCancel()
	{
		SceneManager.SetUpdatePaused("GameScene", false);
		SceneManager.Unload("PauseScene");
	}

	public void OnUISelect(string selectElementName)
	{
		SEPlayer audio = entity.GetComponent<SEPlayer>();
		if (audio)
		{
			audio.Play();
		}
	}

}
