using System;
using System.Runtime.InteropServices;

public class TextRendererTestScript : MonoScript
{

	[SerializeField] string text = "Hello World!";

	public override void Update()
	{
		TextRenderer renderer = entity.GetComponent<TextRenderer>();
		if (renderer)
		{
			renderer.text = text;
		}
	}

}
