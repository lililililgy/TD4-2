using System;
using System.Collections.Generic;



public class GameSceneController : MonoScript
{

	public override void Initialize()
	{

	}

	public override void Update()
	{
		if (CheckInput())
		{
			SceneManager.Add("PauseScene");
			SceneManager.SetUpdatePaused("GameScene", true);
		}

		// ゲームをクリアしたか
		if (CheckClear())
		{
			// クリアシーンへ遷移
			SceneManager.LoadScene("GameClearScene");
		}
	}



	private bool CheckInput()
	{
		if (Input.TriggerKey(KeyCode.Escape))
		{
			return true;
		}

		if (Input.TriggerGamepad(Gamepad.Start))
		{
			return true;
		}

		return false;
	}

	private bool CheckClear()
	{
		ObjectiveSystem objectiveSystem = entity.GetScript<ObjectiveSystem>();
		if (objectiveSystem == null)
		{
			return false;
		}
		return objectiveSystem.IsFinished;
	}

}
