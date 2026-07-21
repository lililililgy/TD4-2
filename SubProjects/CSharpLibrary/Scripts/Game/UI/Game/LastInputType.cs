using System;
using System.Collections.Generic;

/// <summary>
/// 最後に入力したのがKeyboardかGamepadかを判定する
/// 0: Keyboard, 1: Gamepad
/// </summary>
public class LastInputType : MonoScript
{

	int lastInputType = 0;

	public override void Initialize()
	{

	}

	public override void Update()
	{

	}



	int LastInputType()
	{
		if (Input.PressAnyKey())
		{
			lastInputType = 0;
			return lastInputType;
		}

		if (Input.PressGamepadAny())
		{
			lastInputType = 1;
			return lastInputType;
		}

		return lastInputType;
	}

}
