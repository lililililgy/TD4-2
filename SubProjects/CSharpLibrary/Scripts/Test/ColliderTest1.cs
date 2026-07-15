using System;
using System.Collections.Generic;



public class ColliderTest1 : MonoScript
{

	public override void Initialize()
	{

	}

	public override void Update()
	{
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer)
		{
			renderer.color = Vector4.one;
		}
	}


	public override void OnCollisionEnter(Entity collision)
	{
		collision.Destroy();
	}


	public override void OnCollisionStay(Entity entity)
	{
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer)
		{
			renderer.color = Vector4.red;
		}
	}

}
