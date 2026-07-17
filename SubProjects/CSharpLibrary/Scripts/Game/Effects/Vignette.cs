using System;
using System.Collections.Generic;

public class Vignette : MonoScript
{

	[SerializeField] private bool playRequested = false;
	[SerializeField] private float time = 0f;
	[SerializeField] private float maxTime = 1f;
	[SerializeField] private float speed = 1f;
	[SerializeField] private float power = 1f;
	[SerializeField] private float baseAlpha = 1f;
	[SerializeField] private float amplitude = 5f;


	public override void Update()
	{
		if (playRequested)
		{

			time += Time.deltaTime;

			float pulse = Mathf.Pow(Mathf.Abs(Mathf.Sin(time * speed)), power);
			float finalAlpha = baseAlpha + (pulse * amplitude);
			finalAlpha = Mathf.Clamp(finalAlpha, 0f, 1f);

			SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
			if (renderer)
			{
				Vector4 color = renderer.color;
				color.w = finalAlpha;
				renderer.color = color;
			}

			if (time >= maxTime)
			{
				playRequested = false;
				time = 0f;

				if (renderer)
				{
					Vector4 color = renderer.color;
					color.w = 0.0f;
					renderer.color = color;
				}
			}
		}

	}



	public void Play()
	{
		playRequested = true;
	}
}
