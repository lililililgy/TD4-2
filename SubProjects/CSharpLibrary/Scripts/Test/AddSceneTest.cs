using System;
using System.Collections.Generic;

public class AddSceneTest : MonoScript {
	private bool isPaused = false;
	private bool isDrawPaused = false;

	public override void Initialize() {
		
	}

	public override void Update() {
		if(Input.TriggerKey(KeyCode.Space)) {
			System.Console.WriteLine("Adding TestScene...");
			SceneManager.Add("TestScene");
		}

		if(Input.TriggerKey(KeyCode.Escape)) {
			System.Console.WriteLine("Unloading TestScene...");
			SceneManager.Unload("TestScene");
		}

		if(Input.TriggerKey(KeyCode.P)) {
			isPaused = !isPaused;
			System.Console.WriteLine("Set TestScene Update Paused: " + isPaused);
			SceneManager.SetUpdatePaused("TestScene", isPaused);
			System.Console.WriteLine("Current TestScene Update Paused Status: " + SceneManager.IsUpdatePaused("TestScene"));
		}

		if(Input.TriggerKey(KeyCode.D)) {
			isDrawPaused = !isDrawPaused;
			System.Console.WriteLine("Set TestScene Draw Paused: " + isDrawPaused);
			SceneManager.SetDrawPaused("TestScene", isDrawPaused);
			System.Console.WriteLine("Current TestScene Draw Paused Status: " + SceneManager.IsDrawPaused("TestScene"));
		}
	}
}
