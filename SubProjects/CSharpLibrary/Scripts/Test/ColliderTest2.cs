using System;
using System.Collections.Generic;

public class ColliderTest2 : MonoScript {

	[SerializeField] float test = 0.0f;


	public override void Initialize() {
		
	}

	public override void Update() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.one;
		}
	}


	public override void OnCollisionStay(Entity entity) {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.blue;
		}
	}
}
