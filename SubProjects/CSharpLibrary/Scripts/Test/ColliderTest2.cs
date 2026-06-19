using System;
using System.Collections.Generic;

public class ColliderTest2 : MonoScript {
	public override void Initialize() {
		
	}

	public override void Update() {
		MeshRenderer renderer = entity.GetComponent<MeshRenderer>();
		if (renderer) {
			renderer.color = Vector4.one;
		}
	}


	public override void OnCollisionStay(Entity entity) {
		MeshRenderer renderer = entity.GetComponent<MeshRenderer>();
		if (renderer) {
			renderer.color = Vector4.blue;
		}
	}
}
