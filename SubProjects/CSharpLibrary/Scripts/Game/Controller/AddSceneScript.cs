using System;
using System.Collections.Generic;

public class AddSceneScript : MonoScript
{

	[SerializeField] private string sceneName = "TestScene";
	[SerializeField] public bool isAddRequested = false;

	public override void Initialize()
	{

	}

	public override void Update()
	{
		if (isAddRequested)
		{
			SceneManager.Add(sceneName);
			isAddRequested = false;
		}
	}
}
