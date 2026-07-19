using System;

public class RumbleTestScript : MonoScript
{
	private int frameCount_ = 0;

	public override void Update()
	{

		if (Input.TriggerGamepad(Gamepad.A))
		{
			Input.PlayGamepadVibration(12.5f, 0.8f, 0.2f);
		}
	}
}
