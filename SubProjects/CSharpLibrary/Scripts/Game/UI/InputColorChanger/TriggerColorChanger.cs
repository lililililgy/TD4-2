using System;
using System.Collections.Generic;

public class TriggerColorChanger : MonoScript
{

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
		return false;
	}
}
