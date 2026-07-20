using System;
using System.Collections.Generic;

public class KeyColorChanger : MonoScript
{

	[SerializeField] List<KeyCode> keys = new List<KeyCode>();
	[SerializeField] List<Gamepad> pads = new List<Gamepad>();

	[SerializeField] private Vector4 color = Vector4.one;
	Vector4 defaultColor = Vector4.one;

	public override void Initialize()
	{
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer)
		{
			defaultColor = renderer.color;
		}
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
		foreach (KeyCode key in keys)
		{
			if (Input.TriggerKey(key))
			{
				return true;
			}
		}

		foreach (Gamepad pad in pads)
		{
			if (Input.TriggerGamepad(pad))
			{
				return true;
			}
		}

		return false;
	}
}
