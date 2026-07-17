using System;

public class RumbleTestScript : MonoScript
{

	public override void Update()
	{

		if (Input.TriggerGamepad(Gamepad.A))
		{
			Debug.Log("RumbleTest: Gamepad A button triggered.");
			Input.SetGamepadVibration(0.5f, 0.8f);
		}

	}
}
