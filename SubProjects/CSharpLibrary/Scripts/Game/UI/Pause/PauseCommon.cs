using System;
using System.Collections.Generic;

public class PauseCommon : MonoScript
{

	protected bool isSelect = false;
	Vector3 savePos;

	float time = 0f;
	float speed = 1.2f;
	float height = 0.1f;
	Vector4 defaultColor = Vector4.red;
	Vector4 selectedColor = Vector4.green;

	public override void Initialize()
	{
		savePos = transform.position;
	}

	public override void Update()
	{

		Entity parent = entity.parent;
		PauseGroup pauseGroup = parent.GetScript<PauseGroup>();
		if (pauseGroup)
		{
			time = pauseGroup.time;
			speed = pauseGroup.speed;
			defaultColor = pauseGroup.defaultColor;
			selectedColor = pauseGroup.selectedColor;
		}

		if (isSelect)
		{
			float posY = Mathf.Sin(SteppedTimeEasing(Time.time * speed, 1)) * height;
			transform.position = savePos + new Vector3(0, posY, 0);
		}
		else
		{
			transform.position = savePos;
		}


		TextRenderer textRenderer = entity.GetComponent<TextRenderer>();
		if (textRenderer)
		{
			if (isSelect)
			{
				textRenderer.outlineColor = selectedColor;
				textRenderer.color = new Vector4(0, 0, 0, 1);
			}
			else
			{
				textRenderer.outlineColor = defaultColor;
				float v = 0.2f;
				textRenderer.color = new Vector4(v, v, v, 1);
			}
		}
	}


	public static float SteppedTimeEasing(float t, int steps)
	{
		// 進行度tを階段状にする
		float steppedT = Mathf.Floor(t * steps) / steps;
		return steppedT;
	}
}
