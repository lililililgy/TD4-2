using System;
using System.Collections.Generic;

public class GameOver_RestartCheckpoint : BaseUI
{
	void OnSelect()
	{
		isSelect = true;
	}

	void OnDeselect()
	{
		isSelect = false;
	}

	void OnSubmit()
	{
		GameFlow.ResumeFromCheckPoint();
	}
}
