using System;
using System.Collections.Generic;

public class TriggerColorChanger : MonoScript
{

	[SerializeField] private Vector4 color = Vector4.one;
	[SerializeField] private List<Gamepad> pads = new List<Gamepad>();
	Vector4 defaultColor = Vector4.one;

	public override void Initialize()
	{
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer)
		{
			defaultColor = renderer.color;
		}

		pads.Add(Gamepad.LeftShoulder);
		pads.Add(Gamepad.RightShoulder);
	}

	public override void Update()
	{
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer)
		{
			renderer.color = defaultColor;

			if (CheckInput())
			{
				renderer.color = color;
			}
		}

	}


	bool CheckInput()
	{
		if (Input.GamepadLeftTrigger() >= 0.2f)
		{
			return true;
		}

		if (Input.GamepadRightTrigger() >= 0.2f)
		{
			return true;
		}


		foreach (Gamepad pad in pads)
		{
			if (Input.PressGamepad(pad))
			{
				return true;
			}
		}

		return false;
	}
}
