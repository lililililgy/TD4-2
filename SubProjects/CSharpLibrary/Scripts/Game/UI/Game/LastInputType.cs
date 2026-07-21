using System;
using System.Collections.Generic;

/// <summary>
/// 最後に入力したのがKeyboardかGamepadかを判定する
/// 0: Keyboard, 1: Gamepad
/// </summary>
public class LastInputType : MonoScript
{

	public enum InputType
	{
		Keyboard = 0,
		Gamepad = 1
	}

	int lastInputType = 0;
	Entity gamepadEntity = null;
	Entity keyboardEntity = null;

	public override void Initialize()
	{

		gamepadEntity = entity.GetChild(1);
		keyboardEntity = entity.GetChild(0);

		if (gamepadEntity)
		{
			gamepadEntity.enable = false;
		}

		if (keyboardEntity)
		{
			keyboardEntity.enable = true;
		}
	}

	public override void Update()
	{

		int prevInputType = lastInputType;
		lastInputType = GetLastInputType();

		if (lastInputType != -1)
		{
			if (lastInputType != prevInputType)
			{
				switch (lastInputType)
				{
					case (int)InputType.Keyboard:
						if (keyboardEntity)
						{
							keyboardEntity.enable = true;
						}
						if (gamepadEntity)
						{
							gamepadEntity.enable = false;
						}
						break;

					case (int)InputType.Gamepad:
						if (keyboardEntity)
						{
							keyboardEntity.enable = false;
						}
						if (gamepadEntity)
						{
							gamepadEntity.enable = true;
						}
						break;
				}
			}
		}

	}



	int GetLastInputType()
	{
		if (Input.PressAnyKey())
		{
			return (int)InputType.Keyboard;
		}

		if (Input.PressAnyGamepad())
		{
			return (int)InputType.Gamepad;
		}

		return -1;
	}

}
