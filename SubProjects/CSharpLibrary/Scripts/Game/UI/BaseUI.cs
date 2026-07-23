using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class BaseUI : MonoScript
{

	protected bool isSelect = false;
	Vector3 savePos;

	float time = 0f;
	float speed = 1.2f;
	float height = 0.1f;
	Vector4 defaultColor = Vector4.one;
	Vector4 selectedColor = Vector4.red;

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


		// SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		// if (renderer)
		// {
		// 	if (isSelect)
		// 	{
		// 		renderer.color = selectedColor;
		// 	}
		// 	else
		// 	{
		// 		renderer.color = defaultColor;
		// 	}
		// }

		TextRenderer textRenderer = entity.GetComponent<TextRenderer>();
		if (textRenderer)
		{
			if (isSelect)
			{
				textRenderer.color = selectedColor;
			}
			else
			{
				textRenderer.color = defaultColor;
			}
		}
	}


	public static float SteppedTimeEasing(float t, int steps)
	{
		float steppedT = Mathf.Floor(t * steps) / steps;
		return steppedT;
	}

}