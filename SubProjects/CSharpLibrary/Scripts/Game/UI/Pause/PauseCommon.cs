using System;
using System.Collections.Generic;

public class PauseCommon : MonoScript {

	[SerializeField] float time = 0f;

	public override void Initialize() {
		
	}

	public override void Update() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if(renderer) {
			renderer.color = Vector4.red;
		}
	}
}
