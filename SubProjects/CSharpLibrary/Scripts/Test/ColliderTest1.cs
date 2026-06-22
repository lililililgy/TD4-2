using System;
using System.Collections.Generic;



struct TestStruct {
	[SerializeField] float test;
};



public class ColliderTest1 : MonoScript {

	[SerializeField] float test = 0.0f;
	[SerializeField] TestStruct testStruct;

	public override void Initialize() {
		
	}

	public override void Update() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if(renderer) {
			renderer.color = Vector4.one;
		}
	}


	public override void OnCollisionStay(Entity entity) {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.red;
		}
	}

}
