using System;
using System.Collections.Generic;

public class UITest1 : MonoScript {


	public void OnSelect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if(renderer) {
			renderer.color = Vector4.red;
		}
	}

	public void OnDeselect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.one;
		}
	}

}
